#include <Arduino.h>
#include "hardwareDrivers/gsmManager.h"
#include "services/smsService.h"
#include "config.h"

// ==========================================
// UART2 for SIM800L communication
// ==========================================
// ESP32 has 3 hardware UARTs:
//   UART0 = Serial  (USB/debug, pins 1/3 — DO NOT USE)
//   UART1 = Serial1 (default pins conflict with flash — avoid)
//   UART2 = Serial2 (pins 16/17 — perfect for SIM800L)
static HardwareSerial& sim800l = Serial2;

// ==========================================
// Internal helpers
// ==========================================

// Read all available data from SIM800L with timeout
static String readResponse(unsigned long timeout) {
    String response = "";
    unsigned long startTime = millis();

    while (millis() - startTime < timeout) {
        while (sim800l.available()) {
            char c = sim800l.read();
            response += c;
        }
        // If we got "OK" or "ERROR", no need to wait longer
        if (response.indexOf("OK") != -1 || response.indexOf("ERROR") != -1) {
            break;
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    response.trim();
    return response;
}

// ==========================================
// Public API
// ==========================================

void GsmManager::init() {
    // Start UART2 with SIM800L baud rate
    sim800l.begin(SIM800L_BAUD, SERIAL_8N1, SIM800L_RX_PIN, SIM800L_TX_PIN);

    Serial.println("[GSM] UART2 initialized. Waiting for SIM800L...");

    // Give SIM800L time to boot (it needs ~3-5 seconds after power on)
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    // Test connection with AT command
    if (isModuleReady()) {
        Serial.println("[GSM] SIM800L is responding.");
    } else {
        Serial.println("[GSM] SIM800L not responding. Check wiring and power.");
        return;
    }

    // Configure SMS to text mode (easier to parse than PDU mode)
    sendATCommand("AT+CMGF=1");
    Serial.println("[GSM] SMS text mode enabled.");

    // Configure character set to GSM (basic ASCII)
    sendATCommand("AT+CSCS=\"GSM\"");

    // Show sender info when listing SMS
    sendATCommand("AT+CSDH=1");

    // Disable echo (AT commands won't be echoed back)
    sendATCommand("ATE0");

    Serial.println("[GSM] SIM800L fully configured.");
}

bool GsmManager::isModuleReady() {
    // Try AT command up to 3 times
    for (int i = 0; i < 3; i++) {
        String response = sendATCommand("AT", 1000);
        if (response.indexOf("OK") != -1) {
            return true;
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    return false;
}

int GsmManager::getSignalStrength() {
    String response = sendATCommand("AT+CSQ");
    // Response format: "+CSQ: 15,0\r\nOK"
    int csqIndex = response.indexOf("+CSQ:");
    if (csqIndex == -1) return -1;

    int commaIndex = response.indexOf(",", csqIndex);
    if (commaIndex == -1) return -1;

    String rssiStr = response.substring(csqIndex + 6, commaIndex);
    rssiStr.trim();
    return rssiStr.toInt();
}

String GsmManager::sendATCommand(const char* command, unsigned long timeout) {
    // Flush any pending data
    while (sim800l.available()) {
        sim800l.read();
    }

    Serial.printf("[GSM] >> %s\n", command);

    sim800l.println(command);
    String response = readResponse(timeout);

    Serial.printf("[GSM] << %s\n", response.c_str());

    return response;
}

bool GsmManager::sendSMS(const char* number, const char* message) {
    Serial.printf("[GSM] Sending SMS to %s: %s\n", number, message);

    // Step 1: Set recipient number
    char cmd[40];
    snprintf(cmd, sizeof(cmd), "AT+CMGS=\"%s\"", number);
    sim800l.println(cmd);

    // Wait for the ">" prompt from SIM800L
    unsigned long start = millis();
    bool promptReceived = false;
    while (millis() - start < 3000) {
        if (sim800l.available()) {
            char c = sim800l.read();
            if (c == '>') {
                promptReceived = true;
                break;
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    if (!promptReceived) {
        Serial.println("[GSM] No prompt received. SMS send failed.");
        return false;
    }

    // Step 2: Send the message text, followed by Ctrl+Z (0x1A) to send
    sim800l.print(message);
    sim800l.write(0x1A);  // Ctrl+Z = send SMS

    // Step 3: Wait for confirmation
    String response = readResponse(10000);  // SMS can take up to 10 seconds

    if (response.indexOf("+CMGS:") != -1) {
        Serial.println("[GSM] ✅ SMS sent successfully.");
        return true;
    } else {
        Serial.printf("[GSM] SMS send failed. Response: %s\n", response.c_str());
        return false;
    }
}

int GsmManager::checkForNewSMS() {
    // List all unread SMS messages
    String response = sendATCommand("AT+CMGL=\"REC UNREAD\"", 5000);

    // Response format: "+CMGL: 1,"REC UNREAD","+351912345678",...
    int cmglIndex = response.indexOf("+CMGL:");
    if (cmglIndex == -1) {
        return -1;  // No unread messages
    }

    // Extract the index number
    int commaIndex = response.indexOf(",", cmglIndex);
    if (commaIndex == -1) return -1;

    String indexStr = response.substring(cmglIndex + 7, commaIndex);
    indexStr.trim();
    return indexStr.toInt();
}

bool GsmManager::readSMS(int index, char* sender, size_t senderSize,
                         char* message, size_t messageSize) {
    char cmd[20];
    snprintf(cmd, sizeof(cmd), "AT+CMGR=%d", index);
    String response = sendATCommand(cmd, 3000);

    // Response format:
    // +CMGR: "REC UNREAD","+351912345678","","2026/09/30,10:00:00+04"
    // CMD:LOCK:IBIZA2026
    // OK

    // Extract sender phone number
    int firstQuote = response.indexOf("\"+");
    if (firstQuote == -1) {
        // Try without + (local number)
        firstQuote = response.indexOf("\",\"");
        if (firstQuote == -1) return false;
        firstQuote++;  // Skip the comma
    }
    int secondQuote = response.indexOf("\"", firstQuote + 1);
    if (secondQuote == -1) return false;

    String senderStr = response.substring(firstQuote + 1, secondQuote);
    strncpy(sender, senderStr.c_str(), senderSize - 1);
    sender[senderSize - 1] = '\0';

    // Extract message body (after the header line, before "OK")
    int headerEnd = response.indexOf("\n", secondQuote);
    if (headerEnd == -1) return false;

    int okIndex = response.lastIndexOf("OK");
    if (okIndex == -1) okIndex = response.length();

    String msgStr = response.substring(headerEnd + 1, okIndex);
    msgStr.trim();
    strncpy(message, msgStr.c_str(), messageSize - 1);
    message[messageSize - 1] = '\0';

    Serial.printf("[GSM] SMS from %s: %s\n", sender, message);
    return true;
}

bool GsmManager::deleteSMS(int index) {
    char cmd[20];
    snprintf(cmd, sizeof(cmd), "AT+CMGD=%d", index);
    String response = sendATCommand(cmd);
    return response.indexOf("OK") != -1;
}

bool GsmManager::deleteAllSMS() {
    // Delete all SMS: mode 4 = delete all messages
    String response = sendATCommand("AT+CMGD=1,4");
    return response.indexOf("OK") != -1;
}

// ==========================================
// Standalone functions for main.cpp
// ==========================================
void initGSM() {
    GsmManager::init();
}

void taskGSM(void *param) {
    // Poll for new SMS every 5 seconds
    while (true) {
        int smsIndex = GsmManager::checkForNewSMS();

        if (smsIndex >= 0) {
            char sender[20] = {0};
            char message[160] = {0};

            if (GsmManager::readSMS(smsIndex, sender, sizeof(sender),
                                     message, sizeof(message))) {
                
                Serial.printf("[GSM_TASK] New SMS from %s: %s\n", sender, message);
                
                // Parse, validate, and execute the SMS command
                SmsService::processIncomingSMS(sender, message);
            }

            // Delete the processed SMS to free SIM memory
            GsmManager::deleteSMS(smsIndex);
        }

        vTaskDelay(5000 / portTICK_PERIOD_MS);  // Check every 5 seconds
    }
}
