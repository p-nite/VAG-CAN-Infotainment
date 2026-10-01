#pragma once
#include "driver/twai.h"
#include "hardwareDrivers/canSignals.h"

// Standalone functions
void initCAN();
void taskCAN(void *param);

namespace CanService {
    void commandLock();
    void commandUnlock();
    void toggleHazards();
    bool isCarLocked();

    // Funções de lógica pura (testáveis)
    twai_message_t buildLockFrame();
    twai_message_t buildUnlockFrame();
}