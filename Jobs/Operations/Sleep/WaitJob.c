//
// Created by Denis on 1/6/2026.
//

#include "WaitJob.h"

#include <CleanDispatcher.h>
#include <stddef.h>
#include <stdlib.h>

void waitJob(const char *ms) {
    Sleep(strtol(ms, NULL, 10) * 1000);
}
