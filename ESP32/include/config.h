// include/config.h
#pragma once
#include <Arduino.h>

// ==========================================
// Bluetooth — Whitelist
// ==========================================
// Android: Definições → Acerca do telefone → Endereço Bluetooth
#define OWNER_BT_MAC "70:4E:E0:9B:0C:E9"

// Scan timeout (how long to search for the phone)
#define SCAN_TIMEOUT_MS 4000  // 4 seconds

// ==========================================
// SIM800L — GSM Module
// ==========================================
#define SIM800L_RX_PIN    16        // ESP32 RX ← SIM800L TX
#define SIM800L_TX_PIN    17        // ESP32 TX → SIM800L RX
#define SIM800L_BAUD      9600     // SIM800L default baud rate
#define SMS_AUTH_TOKEN     "IBIZA2026"  // Auth token for SMS commands

// ==========================================
// Pinos — CAN Bus
// ==========================================
#define PIN_CAN_TX   GPIO_NUM_5
#define PIN_CAN_RX   GPIO_NUM_4

// ==========================================
// Pinos — Sensores
// ==========================================
#define PIN_TTP223   GPIO_NUM_13   // Touch sensor (wake from deep sleep)

// ==========================================
// FreeRTOS Task Config
// ==========================================
#define TASK_STACK_CAN  4096
#define TASK_STACK_BLE  8192  // BT Classic needs more stack
#define TASK_STACK_GSM  4096