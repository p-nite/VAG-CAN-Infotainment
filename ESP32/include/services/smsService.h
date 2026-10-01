#pragma once
#include <Arduino.h>

// ==========================================
// SMS Service — Service Layer
// ==========================================
// Parses incoming SMS commands, validates auth tokens,
// and triggers actions. Uses GsmManager for sending replies.
//
// Command format: CMD:<ACTION>:<AUTH_TOKEN>
// Examples:
//   CMD:LOCK:IBIZA2026
//   CMD:UNLOCK:IBIZA2026
//   CMD:LIGHTS:IBIZA2026
//   CMD:STATUS:IBIZA2026

enum class SmsCommand {
    LOCK,
    UNLOCK,
    LIGHTS,
    STATUS,
    UNKNOWN
};

struct ParsedSMS {
    SmsCommand command;
    bool authValid;
    char sender[20];
};

namespace SmsService {

    // Parse a raw SMS message into a structured command
    ParsedSMS parseCommand(const char* sender, const char* message);

    // Process an incoming SMS: parse, validate, execute, reply
    void processIncomingSMS(const char* sender, const char* message);

    // Send a status reply to the sender
    void sendStatusReply(const char* number, const char* status);
}
