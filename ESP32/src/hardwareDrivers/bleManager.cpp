#include <Arduino.h>
#include "hardwareDrivers/bleManager.h"
#include "config.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"

// ==========================================
// Internal state for synchronization
// ==========================================
static volatile bool requestFinished = false;
static volatile bool deviceFound = false;

// ==========================================
// GAP Callback
// ==========================================
static void gapCallback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
    if (event == ESP_BT_GAP_READ_REMOTE_NAME_EVT) {
        // This event fires when esp_bt_gap_read_remote_name completes
        if (param->read_rmt_name.stat == ESP_BT_STATUS_SUCCESS) {
            // We got the name, meaning the device is nearby and BT is on!
            deviceFound = true;
            Serial.printf("[BLE] Remote device responded! Name: %s\n", param->read_rmt_name.rmt_name);
        } else {
            // Failed to get name (device off, out of range, or BT disabled)
            deviceFound = false;
            Serial.println("[BLE] Remote device did not respond.");
        }
        requestFinished = true;
    }
}

// ==========================================
// Public API
// ==========================================

void BleManager::init() {
    // Release BLE memory since we only use Classic BT
    esp_bt_controller_mem_release(ESP_BT_MODE_BLE);

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_err_t ret;

    ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK) {
        Serial.printf("[BLE] Controller init failed: %s\n", esp_err_to_name(ret));
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);
    if (ret != ESP_OK) {
        Serial.printf("[BLE] Controller enable failed: %s\n", esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_init();
    if (ret != ESP_OK) {
        Serial.printf("[BLE] Bluedroid init failed: %s\n", esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_enable();
    if (ret != ESP_OK) {
        Serial.printf("[BLE] Bluedroid enable failed: %s\n", esp_err_to_name(ret));
        return;
    }

    // Register the GAP callback
    esp_bt_gap_register_callback(gapCallback);

    // Set a friendly device name (visible to other devices)
    esp_bt_dev_set_device_name("IbizaSmartCar");

    Serial.println("[BLE] Bluetooth Classic initialized (Targeted Paging Mode).");
}

bool BleManager::isDevicePresent(const char* macStr, int timeoutMs) {
    if (macStr == nullptr || strlen(macStr) < 17) return false;

    // Convert string MAC to esp_bd_addr_t (uint8_t[6])
    int mac[6];
    if (sscanf(macStr, "%02x:%02x:%02x:%02x:%02x:%02x", 
               &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) != 6) {
        Serial.println("[BLE] Invalid MAC address format.");
        return false;
    }

    esp_bd_addr_t bda;
    for (int i = 0; i < 6; i++) {
        bda[i] = (uint8_t)mac[i];
    }

    Serial.printf("[BLE] Paging device %s...\n", macStr);

    // Reset flags
    requestFinished = false;
    deviceFound = false;

    // Trigger the remote name request (this is a Direct Paging action)
    esp_err_t ret = esp_bt_gap_read_remote_name(bda);
    if (ret != ESP_OK) {
        Serial.printf("[BLE] Paging failed to start: %s\n", esp_err_to_name(ret));
        return false;
    }

    // Wait for the callback to set requestFinished
    unsigned long startTime = millis();
    while (!requestFinished && (millis() - startTime < timeoutMs)) {
        vTaskDelay(50 / portTICK_PERIOD_MS); // Yield to FreeRTOS
    }

    // If timeout reached
    if (!requestFinished) {
        Serial.println("[BLE] Paging timeout (device out of range).");
        return false;
    }

    return deviceFound;
}

// ==========================================
// Standalone functions for main.cpp
// ==========================================
void initBLE() {
    BleManager::init();
}

void taskBLE(void *param) {
    while (true) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
