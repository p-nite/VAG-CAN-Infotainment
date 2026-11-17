#include <Arduino.h>
#include "services/smsService.h"
#include "hardwareDrivers/gsmManager.h"
#include "hardwareDrivers/canManager.h"
#include "config.h"
#include <string.h>

// ==========================================
// Command Parsing (Testable pure logic)
// ==========================================

ParsedSMS SmsService::parseCommand(const char* sender, const char* message) {
    ParsedSMS result;
    result.command = SmsCommand::UNKNOWN;
    result.authValid = false;
    strncpy(result.sender, sender, sizeof(result.sender) - 1);
    result.sender[sizeof(result.sender) - 1] = '\0';

    if (message == nullptr || strlen(message) == 0) {
        return result;
    }

    // Make a mutable copy for tokenizing
    char msgCopy[160];
    strncpy(msgCopy, message, sizeof(msgCopy) - 1);
    msgCopy[sizeof(msgCopy) - 1] = '\0';

    // Expected format: CMD:ACTION:TOKEN
    // Split by ':'
    char* prefix = strtok(msgCopy, ":");
    char* action = strtok(NULL, ":");
    char* token  = strtok(NULL, ":");

    // Validate prefix
    if (prefix == nullptr || strcmp(prefix, "CMD") != 0) {
        Serial.println("[SMS_SRV] Invalid prefix (expected CMD).");
        return result;
    }

    // Validate action
    if (action == nullptr) {
        Serial.println("[SMS_SRV] Missing action.");
        return result;
    }

    if (strcmp(action, "LOCK") == 0)        result.command = SmsCommand::LOCK;
    else if (strcmp(action, "UNLOCK") == 0)  result.command = SmsCommand::UNLOCK;
    else if (strcmp(action, "LIGHTS") == 0)  result.command = SmsCommand::LIGHTS;
    else if (strcmp(action, "STATUS") == 0)  result.command = SmsCommand::STATUS;
    else {
        Serial.printf("[SMS_SRV] Unknown action: %s\n", action);
        return result;
    }

    // Validate auth token
    if (token != nullptr && strcmp(token, SMS_AUTH_TOKEN) == 0) {
        result.authValid = true;
    } else {
        Serial.println("[SMS_SRV] Invalid auth token.");
    }

    return result;
}

// ==========================================
// Process & Execute
// ==========================================

void SmsService::processIncomingSMS(const char* sender, const char* message) {
    Serial.printf("[SMS_SRV] Processing SMS from %s: %s\n", sender, message);

    ParsedSMS parsed = parseCommand(sender, message);

    // ==========================================
    // SMS Cost Optimization:
    // Only reply with SMS on SUCCESSFUL commands.
    // Invalid auth or unknown commands are silently
    // dropped (logged to Serial only) to avoid
    // wasting SMS credits on spam or typos.
    // ==========================================

    // Reject if auth failed — NO SMS reply (save credits)
    if (!parsed.authValid) {
        Serial.printf("[SMS_SRV] Auth failed from %s. Ignoring (no SMS reply).\n", sender);
        return;
    }

    // Reject unknown commands — NO SMS reply (save credits)
    if (parsed.command == SmsCommand::UNKNOWN) {
        Serial.printf("[SMS_SRV] Unknown command from %s. Ignoring (no SMS reply).\n", sender);
        return;
    }

    // Execute the command — ONLY successful actions get a reply
    switch (parsed.command) {

        case SmsCommand::LOCK:
            Serial.println("[SMS_SRV] Executing LOCK...");
            CanService::commandLock();
            sendStatusReply(sender, "OK:LOCKED");
            break;

        case SmsCommand::UNLOCK:
            Serial.println("[SMS_SRV] Executing UNLOCK...");
            CanService::commandUnlock();
            sendStatusReply(sender, "OK:UNLOCKED");
            break;

        case SmsCommand::LIGHTS:
            Serial.println("[SMS_SRV] Executing LIGHTS...");
            CanService::toggleHazards();
            sendStatusReply(sender, "OK:LIGHTS");
            break;

        case SmsCommand::STATUS: {
            Serial.println("[SMS_SRV] Executing STATUS query...");
            bool locked = CanService::isCarLocked();
            int signal = GsmManager::getSignalStrength();
            char reply[80];
            snprintf(reply, sizeof(reply), "OK:%s:SIG%d",
                     locked ? "LOCKED" : "UNLOCKED", signal);
            sendStatusReply(sender, reply);
            break;
        }

        default:
            Serial.println("[SMS_SRV] Unhandled command (should not happen).");
            break;
    }
}

void SmsService::sendStatusReply(const char* number, const char* status) {
    Serial.printf("[SMS_SRV] Replying to %s: %s\n", number, status);
    GsmManager::sendSMS(number, status);
}
