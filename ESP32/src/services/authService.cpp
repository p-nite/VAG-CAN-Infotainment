#include <Arduino.h>
#include "services/authService.h"
#include "hardwareDrivers/bleManager.h"
#include "config.h"
#include <string.h>

// ==========================================
// Whitelist of authorized MAC addresses
// ==========================================
static const char* WHITELIST[] = {
    OWNER_BT_MAC,
    // Add more devices here if needed
};
static const int WHITELIST_SIZE = sizeof(WHITELIST) / sizeof(WHITELIST[0]);

// ==========================================
// Public API
// ==========================================

void AuthService::init() {
    // BLE hardware is initialized by BleManager::init() via initBLE()
    Serial.println("[AUTH] Auth service initialized.");
}

bool AuthService::isOwnerNearby() {
    Serial.println("[AUTH] Checking if owner is nearby...");

    // Iterate through the whitelist and check each MAC explicitly
    for (int i = 0; i < WHITELIST_SIZE; i++) {
        // We use SCAN_TIMEOUT_MS as the timeout per device check
        if (BleManager::isDevicePresent(WHITELIST[i], SCAN_TIMEOUT_MS)) {
            Serial.printf("[AUTH] ✅ Authorized device found: %s\n", WHITELIST[i]);
            return true;
        }
    }

    Serial.println("[AUTH] ❌ No authorized device found.");
    return false;
}

bool AuthService::isMAConWhitelist(const char* foundMAC) {
    if (foundMAC == nullptr) return false;

    for (int i = 0; i < WHITELIST_SIZE; i++) {
        if (strcmp(foundMAC, WHITELIST[i]) == 0) {
            return true;
        }
    }
    return false;
}