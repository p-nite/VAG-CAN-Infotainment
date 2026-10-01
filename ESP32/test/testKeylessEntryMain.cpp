// test/test_lock_system/test_main.cpp
#include <Arduino.h>
#include <unity.h>
#include "core/lockSystem.h"
#include "services/authService.h"
#include "hardwareDrivers/canManager.h"
#include "hardwareDrivers/canSignals.h"
#include "config.h"

// ============================================================
// TESTES DA LÓGICA DE DECISÃO (LockAction)
// ============================================================

void test_owner_nearby_and_car_locked_should_unlock(void) {
    // Cenário: O dono tocou no sensor, o carro está trancado
    // Esperado: Destrancar
    LockAction action = decideLockAction(true, true);
    TEST_ASSERT_EQUAL(LockAction::DO_UNLOCK, action);
}

void test_owner_nearby_and_car_unlocked_should_lock(void) {
    // Cenário: O dono tocou no sensor, o carro está destrancado
    // Esperado: Trancar
    LockAction action = decideLockAction(true, false);
    TEST_ASSERT_EQUAL(LockAction::DO_LOCK, action);
}

void test_owner_not_nearby_and_car_locked_should_do_nothing(void) {
    // Cenário: Alguém tocou no sensor mas o telemóvel do dono não está perto
    // Esperado: Não fazer nada (segurança!)
    LockAction action = decideLockAction(false, true);
    TEST_ASSERT_EQUAL(LockAction::DO_NOTHING, action);
}

void test_owner_not_nearby_and_car_unlocked_should_do_nothing(void) {
    // Cenário: Alguém tocou mas o dono não está perto, carro aberto
    // Esperado: Não fazer nada (não trancar nem destrancar sem auth)
    LockAction action = decideLockAction(false, false);
    TEST_ASSERT_EQUAL(LockAction::DO_NOTHING, action);
}

// ============================================================
// TESTES DE AUTENTICAÇÃO (MAC Whitelist)
// ============================================================

void test_known_mac_is_authorized(void) {
    // O MAC do dono (definido no config.h) deve ser aceite
    TEST_ASSERT_TRUE(AuthService::isMAConWhitelist(OWNER_BT_MAC));
}

void test_unknown_mac_is_rejected(void) {
    // Um MAC aleatório não deve ser aceite
    TEST_ASSERT_FALSE(AuthService::isMAConWhitelist("11:22:33:44:55:66"));
}

void test_all_positions_reject_when_modified(void) {
    // Testar que alterar QUALQUER posição do MAC resulta em rejeição
    // Posições dos hex digits no MAC: 0,1, 3,4, 6,7, 9,10, 12,13, 15,16
    int hexPositions[] = {0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15, 16};

    for (int i = 0; i < 12; i++) {
        char modifiedMAC[18];
        strncpy(modifiedMAC, OWNER_BT_MAC, sizeof(modifiedMAC));

        // Incrementar o carácter nesta posição
        char c = modifiedMAC[hexPositions[i]];
        if (c >= '0' && c <= '8')      c += 1;
        else if (c == '9')             c = 'A';
        else if (c >= 'A' && c <= 'E') c += 1;
        else if (c == 'F')            c = '0';

        modifiedMAC[hexPositions[i]] = c;

        TEST_ASSERT_FALSE_MESSAGE(
            AuthService::isMAConWhitelist(modifiedMAC),
            modifiedMAC  // Se falhar, mostra qual MAC causou o problema
        );
    }
}

void test_null_mac_is_rejected(void) {
    // Proteção contra NULL pointer
    TEST_ASSERT_FALSE(AuthService::isMAConWhitelist(nullptr));
}

void test_empty_mac_is_rejected(void) {
    // String vazia não deve ser aceite
    TEST_ASSERT_FALSE(AuthService::isMAConWhitelist(""));
}

// ============================================================
// TESTES DAS CAN FRAMES (Construção das mensagens)
// ============================================================

void test_lock_frame_has_correct_id(void) {
    // A frame de trancar deve ter o CAN ID do BCM
    twai_message_t msg = CanService::buildLockFrame();
    TEST_ASSERT_EQUAL_UINT32(CanIds::BCM_CONTROL, msg.identifier);
}

void test_lock_frame_has_correct_value(void) {
    // O byte de comando deve ter o valor de LOCK
    twai_message_t msg = CanService::buildLockFrame();
    TEST_ASSERT_EQUAL_UINT8(CanCmds::VAL_LOCK, msg.data[CanCmds::BYTE_LOCK_CMD]);
}

void test_unlock_frame_has_correct_id(void) {
    // A frame de destrancar deve ter o mesmo CAN ID
    twai_message_t msg = CanService::buildUnlockFrame();
    TEST_ASSERT_EQUAL_UINT32(CanIds::BCM_CONTROL, msg.identifier);
}

void test_unlock_frame_has_correct_value(void) {
    // O byte de comando deve ter o valor de UNLOCK
    twai_message_t msg = CanService::buildUnlockFrame();
    TEST_ASSERT_EQUAL_UINT8(CanCmds::VAL_UNLOCK, msg.data[CanCmds::BYTE_LOCK_CMD]);
}

void test_lock_frame_is_standard_frame(void) {
    // Deve ser standard (11-bit ID), não extended (29-bit)
    twai_message_t msg = CanService::buildLockFrame();
    TEST_ASSERT_EQUAL_UINT8(0, msg.extd);
}

void test_lock_frame_dlc_is_8(void) {
    // Data Length Code deve ser 8 (VAG usa sempre 8 bytes)
    twai_message_t msg = CanService::buildLockFrame();
    TEST_ASSERT_EQUAL_UINT8(8, msg.data_length_code);
}

void test_lock_frame_unused_bytes_are_zero(void) {
    // Bytes não usados no frame devem ser 0x00
    twai_message_t msg = CanService::buildLockFrame();
    for (int i = 0; i < 8; i++) {
        if (i != CanCmds::BYTE_LOCK_CMD) {
            TEST_ASSERT_EQUAL_UINT8(0x00, msg.data[i]);
        }
    }
}

void test_lock_and_unlock_values_are_different(void) {
    // Garantir que não confundimos lock com unlock
    TEST_ASSERT_NOT_EQUAL(CanCmds::VAL_LOCK, CanCmds::VAL_UNLOCK);
}

// ============================================================
// TESTES DE CONFIGURAÇÃO
// ============================================================

void test_scan_timeout_is_reasonable(void) {
    // O timeout do scan deve ser entre 2 e 10 segundos
    TEST_ASSERT_GREATER_OR_EQUAL(2000, SCAN_TIMEOUT_MS);
    TEST_ASSERT_LESS_OR_EQUAL(10000, SCAN_TIMEOUT_MS);
}

void test_owner_mac_is_configured(void) {
    // O MAC do dono não deve estar vazio
    TEST_ASSERT_GREATER_THAN(0, strlen(OWNER_BT_MAC));
}

void test_owner_mac_has_correct_format(void) {
    // O MAC deve ter 17 caracteres (XX:XX:XX:XX:XX:XX)
    TEST_ASSERT_EQUAL(17, strlen(OWNER_BT_MAC));
}

// ============================================================
// RUNNER
// ============================================================

void setUp(void) {}
void tearDown(void) {}

void setup() {
    delay(2000);
    UNITY_BEGIN();

    // Lógica de decisão
    RUN_TEST(test_owner_nearby_and_car_locked_should_unlock);
    RUN_TEST(test_owner_nearby_and_car_unlocked_should_lock);
    RUN_TEST(test_owner_not_nearby_and_car_locked_should_do_nothing);
    RUN_TEST(test_owner_not_nearby_and_car_unlocked_should_do_nothing);

    // Autenticação
    RUN_TEST(test_known_mac_is_authorized);
    RUN_TEST(test_unknown_mac_is_rejected);
    RUN_TEST(test_all_positions_reject_when_modified);
    RUN_TEST(test_null_mac_is_rejected);
    RUN_TEST(test_empty_mac_is_rejected);

    // CAN Frames
    RUN_TEST(test_lock_frame_has_correct_id);
    RUN_TEST(test_lock_frame_has_correct_value);
    RUN_TEST(test_unlock_frame_has_correct_id);
    RUN_TEST(test_unlock_frame_has_correct_value);
    RUN_TEST(test_lock_frame_is_standard_frame);
    RUN_TEST(test_lock_frame_dlc_is_8);
    RUN_TEST(test_lock_frame_unused_bytes_are_zero);
    RUN_TEST(test_lock_and_unlock_values_are_different);

    // Configuração
    RUN_TEST(test_scan_timeout_is_reasonable);
    RUN_TEST(test_owner_mac_is_configured);
    RUN_TEST(test_owner_mac_has_correct_format);

    UNITY_END();
}

void loop() {}