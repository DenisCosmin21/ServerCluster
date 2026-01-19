//
// Created by Denis on 1/17/2026.
//

#include "ConvolutionHandler.h"

#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include "../../../Config/config.h"
#include "Operations/Convolution/ComputeConvolution.h"
#include "ReadJobHandler.h"

static size_t getKernelSize(const char *convType) {
    if(strcmp(convType, "sharpen") == 0)
        return 3;
    if(strcmp(convType, "horizontal-edge") == 0)
        return 3;
    if(strcmp(convType, "box-blur") == 0)
        return 3;
    if(strcmp(convType, "gaussian-blur") == 0)
        return 3;

    return 3;
}

static size_t getPaddingSize(const char *convType) {
    return (getKernelSize(convType) - 1) / 2;
}

static void readRows(FILE *file, char *buffer, const size_t startRow,const size_t lastRow, const imageHeader_t *headerInfo) {
    size_t totalPadding = headerInfo->paddedWidth - headerInfo->width;

    size_t row_padded = (headerInfo->width * 3 + 3) & (~3);

    char *row = malloc(row_padded);

    if(row == NULL) {
        perror("malloc");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    size_t sidePadding = totalPadding / 2;

    size_t startCol = sidePadding;

    for (size_t y = startRow; y < lastRow; y++)
    {
        fread(row, sizeof(char), row_padded, file);

        if(totalPadding > 0) {
            buffer[0] = 0;
            buffer[headerInfo->width + sidePadding] = 0;
        }

        for (size_t x = startCol; x < headerInfo->width; x++)
        {
            size_t target_pos = ((lastRow - 1 - y) * headerInfo->width + x) * 3;
            size_t source_pos = x * 3;
            buffer[target_pos] = row[source_pos];
            buffer[target_pos + 1] = row[source_pos + 1];
            buffer[target_pos + 2] = row[source_pos + 2];
        }
    }

    free(row);
}

static void parallelConvolutionHandler(FILE *file, job_t job, const imageHeader_t *headerInfo) {
    size_t totalPadding = headerInfo->paddedHeight - headerInfo->height;

    size_t sidePadding = totalPadding / 2;

    size_t jobsCreated = headerInfo->paddedHeight / IMAGE_SIZE_THRESHOLD;

    if(jobsCreated > MAX_JOBS_PER_JOB)
        jobsCreated = MAX_JOBS_PER_JOB;

    size_t chunkSize = headerInfo->paddedHeight / jobsCreated;

    size_t lastChunk = headerInfo->paddedHeight % jobsCreated + chunkSize;

    size_t startRow = headerInfo->paddedHeight;

    size_t lastRow = headerInfo->paddedHeight;

    char *buffer = NULL;

    size_t chunk = 0;

    while(chunk <= jobsCreated) {
        char *jobHeader = malloc(strlen(headerInfo->type) + 40 + 40); //Give size for widht and height

        if(jobHeader == NULL) {
            perror("malloc");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        jobHeader[0] = '\0';

        if(chunk == jobsCreated) {
            buffer = malloc(headerInfo->paddedWidth * lastChunk * 3); //We use a continous char buffer
            sprintf(jobHeader, "%llu %llu %llu %llu %llu %llu %s", headerInfo->width, lastChunk, headerInfo->paddedWidth, lastChunk, headerInfo->width, headerInfo->height, headerInfo->type);
            startRow -= lastChunk;
        }
        else {
            buffer = malloc(headerInfo->paddedWidth * chunkSize * 3);
            sprintf(jobHeader, "%llu %llu %llu %llu %llu %llu %s", headerInfo->width, chunkSize, headerInfo->paddedWidth, chunkSize, headerInfo->width, headerInfo->height, headerInfo->type);
            startRow -= chunkSize;
        }

        if(totalPadding > 0) {
            memset(buffer, 0, headerInfo->width + totalPadding);
        }

        readRows(file, buffer, startRow, lastRow, headerInfo);

        if(chunk == jobsCreated)
            lastRow -= lastChunk;
        else
            lastRow -= chunkSize;

        job_t jobToAdd = newJob(CONVOLUTION, buffer, jobHeader, job->jobId, chunk);

        enqueueJob(jobToAdd);

        chunk++;
    }
}

static void serialConvolutionHandler(FILE *file, job_t job, const imageHeader_t *headerInfo) {
    char *jobHeader = malloc(strlen(headerInfo->type) + 40 + 40); //Give size for widht and height

    if(jobHeader == NULL) {
        perror("malloc");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    jobHeader[0] = '\0';

    size_t totalPadding = headerInfo->paddedWidth - headerInfo->width;

    size_t sidePadding = totalPadding / 2;

    sprintf(jobHeader, "%llu %llu %llu %llu %llu %llu %s", headerInfo->width, headerInfo->height, headerInfo->paddedWidth, headerInfo->paddedHeight, headerInfo->width, headerInfo->height, headerInfo->type);

    char *buffer = malloc((headerInfo->paddedWidth) * (headerInfo->paddedHeight) * 3); //We use a continous char buffer

    if(buffer == NULL) {
        perror("Error allocating memory");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if(totalPadding > 0) {
        memset(buffer, 0, headerInfo->width + totalPadding);
    }

    readRows(file, buffer, sidePadding, headerInfo->height, headerInfo);

    job_t jobToAdd = newJob(CONVOLUTION, buffer, jobHeader, job->jobId, 0);

    enqueueJob(jobToAdd);
}

void convolutionHandler(job_t job) {
    char *params = job->params;

    char *convTypeCopy = strtok(params, " ");

    char *fileName = strtok(NULL, " ");

    FILE *file = fopen(fileName, "rb");

    if (file == NULL) {
        perror("Error opening file");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    unsigned char header[54];
    if (fread(header, sizeof(unsigned char), 54, file) != 54)
    {
        perror(  "Error: Invalid BMP header\n");
        fclose(file);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (header[0] != 'B' || header[1] != 'M') {
        perror(  "Error: Not a valid BMP file\n");
        fclose(file);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int dataOffset = *(int *)&header[10]; // The actual start of pixel data
    fseek(file, dataOffset, SEEK_SET);

    int width = *(int *)&header[18];
    int height = *(int *)&header[22];

    int bitsPerPixel = *(short *)&header[28];

    if(bitsPerPixel != 24) {
        perror( "Error: Not a valid BMP file\n");
        fclose(file);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    size_t totalPadding = getPaddingSize(convTypeCopy);

    imageHeader_t headerInfo;
    headerInfo.width = width;
    headerInfo.height = height;
    headerInfo.type = convTypeCopy;
    headerInfo.paddedWidth = width + totalPadding * 2;
    headerInfo.paddedHeight = height + totalPadding * 2;

    if(height + totalPadding <= IMAGE_SIZE_THRESHOLD) {
        serialConvolutionHandler(file, job, &headerInfo);
    }
    else {
        parallelConvolutionHandler(file, job, &headerInfo);
    }

    fclose(file);
    destructJob(job);
}

void saveConvolution(const char *filename, const char *data, const char *params)
{
    // 1. Open in rb+ to allow reading the header AND writing at specific offsets
    FILE *f = fopen(filename, "rb");
    int headerExists = 1;

    if (!f) {
        f = fopen(filename, "ab");
        headerExists = 0;
        if (!f) {
            perror("Error: Could not open/create file");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
    else {
        fclose(f);
        f = fopen(filename, "ab");
    }

    imageHeader_t headerInfo = getHeaderInfo(params);

    size_t row_padded = (headerInfo.totalWidth * 3 + 3) & (~3);

    unsigned char header[54] = {
        'B', 'M',    // Signature
        0, 0, 0, 0,  // File size
        0, 0, 0, 0,  // Reserved
        54, 0, 0, 0, // Data offset
        40, 0, 0, 0, // Header size
        0, 0, 0, 0,  // Width
        0, 0, 0, 0,  // Height
        1, 0,        // Planes
        24, 0,       // Bits per pixel
        0, 0, 0, 0,  // Compression (none)
        0, 0, 0, 0,  // Image size (can be 0 for no compression)
        0, 0, 0, 0,  // X pixels per meter
        0, 0, 0, 0,  // Y pixels per meter
        0, 0, 0, 0,  // Total colors
        0, 0, 0, 0   // Important colors
    };

    *(int *)&header[2] = 54 + (int)row_padded * (int)headerInfo.totalHeight;
    *(int *)&header[18] = (int)headerInfo.totalWidth;
    *(int *)&header[22] = (int)headerInfo.totalHeight;

    if(!headerExists) {
        fwrite(header, sizeof(unsigned char), 54, f);
    }

    unsigned char *row = (unsigned char *)calloc(1, row_padded);

    if (!row)
    {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(f);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }


    for (int y = 0; y < headerInfo.height; y++)
    {
        for (int x = 0; x < (int)headerInfo.width; x++)
        {
            int src_idx = ((headerInfo.height - 1 - y) * headerInfo.width + x) * 3;
            row[x * 3 + 0] = data[src_idx + 0];
            row[x * 3 + 1] = data[src_idx + 1];
            row[x * 3 + 2] = data[src_idx + 2];
        }
        fwrite(row, 1, row_padded, f);
    }


    free(row);
    fflush(f);
    fclose(f);
}
