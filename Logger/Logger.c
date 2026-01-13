//
// Created by Denis on 1/13/2026.
//

#include "Logger.h"
#include "../Config/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

static FILE* logFile = NULL;
static CRITICAL_SECTION logMutex;

void initLogger(void) {
    logFile = fopen(LOG_FILE, "a");

    if(logFile == NULL) {
        perror("Eroare deschidre loger");
        exit(-1);
    }

    InitializeCriticalSection(&logMutex);
}

static void generateTimeStamp(void) {
    time_t now;
    struct tm *timeInfo;
    char buffer[25];

    time(&now);

    timeInfo = localtime(&now);

    strftime(buffer, sizeof(buffer), "[%Y-%m-%d %H:%M:%S]", timeInfo);

    fprintf(logFile, "%s ", buffer);
}

void logData(const char* data) {
    if (logFile == NULL) {
        return;
    }

    EnterCriticalSection(&logMutex);

    generateTimeStamp();

    fprintf(logFile, "%s\n", data);

    LeaveCriticalSection(&logMutex);

    fflush(logFile);
}

void freeLogger(void) {
    fclose(logFile);
}