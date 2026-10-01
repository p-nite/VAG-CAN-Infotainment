// src/core/lock_system.cpp
#include "core/lockSystem.h"
#include "services/authService.h"
#include "hardwareDrivers/canManager.h"
#include "config.h"
#include <esp_sleep.h>
#include "core/lockSystem.h"
#include <HardwareSerial.h>

LockAction decideLockAction(bool ownerNearby, bool isCurrentlyLocked) {
    if (!ownerNearby) {
        return LockAction::DO_NOTHING;
    }
    if (isCurrentlyLocked) {
        return LockAction::DO_UNLOCK;
    } else {
        return LockAction::DO_LOCK;
    }
}

void LockSystem::handleTouchEvent() {
    Serial.println("[LOCK] Touch detected! Processing...");

    // 1. Checks if the owner's device is nearby
    if (!AuthService::isOwnerNearby()) {
        Serial.println("[LOCK] Access denied. Returning to sleep...");
        enterDeepSleep();
        return;
    }

    // 2. Verifies the current lock state of the car
    bool isLocked = CanService::isCarLocked();

    // 3. Switches the lock state
    if (isLocked) {
        Serial.println("[LOCK] Locked -> Unlocking...");
        CanService::commandUnlock();
    } else {
        Serial.println("[LOCK] Unlocked -> Locking...");
        CanService::commandLock();
    }

    Serial.println("[LOCK] Done! Sleeping...");
    enterDeepSleep();
}

void LockSystem::enterDeepSleep() {
    // Para o teste de bancada, vamos desativar o deep sleep
    // senão as tarefas GSM e CAN morrem e o ESP32 desliga-se.
    // esp_sleep_enable_ext0_wakeup(PIN_TTP223, 1);
    
    Serial.println("[LOCK] (Bench Test Mode) Standing by instead of Deep Sleep...");
    vTaskDelay(2000 / portTICK_PERIOD_MS); // Evita spam se ficares a carregar
}