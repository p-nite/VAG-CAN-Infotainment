#include <Arduino.h>
#include "hardwareDrivers/canManager.h"

// TODO: Implementar inicialização real do TWAI
void initCAN() {}
void taskCAN(void *param) { vTaskDelete(NULL); }

namespace CanService {

// ==========================================
// Função genérica para enviar comandos
// ==========================================
static void sendComfortCommand(uint8_t byteIndex, uint8_t value) {
    twai_message_t msg;
    msg.identifier = CanIds::BCM_CONTROL;
    msg.extd = 0;
    msg.rtr = 0;
    msg.data_length_code = 8;

    for(int i = 0; i < 8; i++) {
        msg.data[i] = 0x00; 
    }
    msg.data[byteIndex] = value;
    twai_transmit(&msg, pdMS_TO_TICKS(100));
}

// ==========================================
// Funções de construção de frames (TESTÁVEIS)
// ==========================================
twai_message_t buildLockFrame() {
    twai_message_t msg = {};
    msg.identifier = CanIds::BCM_CONTROL;
    msg.extd = 0;
    msg.rtr = 0;
    msg.data_length_code = 8;
    
    for (int i = 0; i < 8; i++) msg.data[i] = 0x00;
    msg.data[CanCmds::BYTE_LOCK_CMD] = CanCmds::VAL_LOCK;
    
    return msg;
}

twai_message_t buildUnlockFrame() {
    twai_message_t msg = {};
    msg.identifier = CanIds::BCM_CONTROL;
    msg.extd = 0;
    msg.rtr = 0;
    msg.data_length_code = 8;
    
    for (int i = 0; i < 8; i++) msg.data[i] = 0x00;
    msg.data[CanCmds::BYTE_LOCK_CMD] = CanCmds::VAL_UNLOCK;
    
    return msg;
}

// ==========================================
// Funções de comando (HARDWARE)
// ==========================================
void commandLock() {
    twai_message_t msg = buildLockFrame();
    twai_transmit(&msg, pdMS_TO_TICKS(100));
}

void commandUnlock() {
    twai_message_t msg = buildUnlockFrame();
    twai_transmit(&msg, pdMS_TO_TICKS(100));
}

void lockDoors() {
    sendComfortCommand(CanCmds::BYTE_LOCK_CMD, CanCmds::VAL_LOCK);
}

void unlockDoors() {
    sendComfortCommand(CanCmds::BYTE_LOCK_CMD, CanCmds::VAL_UNLOCK);
}

void toggleHazards() {
    sendComfortCommand(CanCmds::BYTE_LIGHTS, CanCmds::VAL_HAZARD_ON);
}

bool isCarLocked() {
    // TODO: Implementar leitura do estado real via CAN
    return false;
}

} // namespace CanService