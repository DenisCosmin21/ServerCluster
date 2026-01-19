//
// Created by Denis on 12/27/2025.
//

#ifndef WORKER_H
#define WORKER_H
#include <mpi.h>

#include "../Utilities/Utilities.h"
char *listenForData(MPI_Status *status);

void runWorker(void);

void exitFailedWorker(void);
#endif //WORKER_H
