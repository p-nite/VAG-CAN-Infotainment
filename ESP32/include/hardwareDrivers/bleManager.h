#pragma once
#include <Arduino.h>

// ==========================================
// BLE Manager — Hardware Driver
// ==========================================
// Handles Bluetooth Classic initialization and 
// targeted device presence checks using Direct Paging
// (Remote Name Request) to bypass Android's privacy limits.

namespace BleManager {

    // Initialize Bluetooth Classic controller
    void init();

    // Check if a specific device is nearby by doing a direct
    // name request. This works even if the phone is NOT discoverable
    // and is locked in the user's pocket.
    // Blocks for up to timeoutMs (default 2000).
    // Returns true if device responds, false if timeout/error.
    bool isDevicePresent(const char* macStr, int timeoutMs = 2000);

}

// Standalone functions for FreeRTOS task management
void initBLE();
void taskBLE(void *param);
