//
// Created by Denis on 1/11/2026.
//

#ifndef EXECUTEJOBHANDLER_H
#define EXECUTEJOBHANDLER_H
#include "../../Job.h"

void executeJobHandler(jobType_t jobType, char *params);

char *executeSerialJobHandler(jobType_t jobType, char *params, char *additionalParams);

void cleanupWorker(void);
#endif //EXECUTEJOBHANDLER_H
