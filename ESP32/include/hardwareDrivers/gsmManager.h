#pragma once
#include <Arduino.h>

// ==========================================
// GSM Manager — Hardware Driver
// ==========================================
// Handles UART communication with the SIM800L module.
// Sends AT commands, reads responses, sends/receives SMS.
// Does NOT know about command parsing or authentication
// — that's the SmsService's job.

namespace GsmManager {

    // Initialize UART2 connection to SIM800L
    void init();

    // Check if SIM800L is responding to AT commands
    bool isModuleReady();

    // Get signal strength (0-31, 99 = unknown)
    int getSignalStrength();

    // Send a raw AT command and wait for response
    // Returns the full response string
    // timeout: max wait time in milliseconds
    String sendATCommand(const char* command, unsigned long timeout = 2000);

    // Send an SMS message to a phone number
    // number format: "+351912345678" (international)
    bool sendSMS(const char* number, const char* message);

    // Check if there are unread SMS messages
    // Returns the index of the first unread SMS, or -1 if none
    int checkForNewSMS();

    // Read an SMS at a specific index
    // Fills sender and message buffers
    // Returns true if successfully read
    bool readSMS(int index, char* sender, size_t senderSize,
                 char* message, size_t messageSize);

    // Delete an SMS at a specific index
    bool deleteSMS(int index);

    // Delete all SMS messages
    bool deleteAllSMS();
}

// Standalone functions for FreeRTOS task management
void initGSM();
void taskGSM(void *param);
