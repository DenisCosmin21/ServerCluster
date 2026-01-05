//
// Created by Denis on 12/26/2025.
//

#include "JobDispatcher.h"
#include <Windows.h>
#include "CleanDispatcher.h"
#include "Initialize.h"

void runDispatcher(void) {
    HANDLE threads[4];

    initializeDispatcher(threads);

    cleanDispatcher(threads);
}