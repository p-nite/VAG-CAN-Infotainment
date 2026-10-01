#pragma once
#include <stdint.h>

// ==========================================
// CAN IDs (Identificadores das mensagens)
// ==========================================
namespace CanIds {
    const uint32_t BCM_CONTROL = 0x291;  // Exemplo: ID para mandar comandos ao BCM
    const uint32_t DOOR_STATUS = 0x420;  // Exemplo: ID onde o BCM diz se as portas estão abertas
}

// ==========================================
// Comandos do BCM (Bytes e Valores)
// ==========================================
namespace CanCmds {
    // Vamos assumir que o comando de fecho vai no Data[0]
    const uint8_t BYTE_LOCK_CMD = 0; 
    const uint8_t VAL_LOCK      = 0x01; // Valor para trancar
    const uint8_t VAL_UNLOCK    = 0x02; // Valor para destrancar

    // Vamos assumir que as luzes vão no Data[1]
    const uint8_t BYTE_LIGHTS   = 1;
    const uint8_t VAL_HAZARD_ON = 0x05; 
}

// ==========================================
// Leitura de Estado (Máscaras de Bits)
// ==========================================
// Muitas vezes no CAN, o estado de 4 portas vem no mesmo byte (ex: Data[0])
namespace CanMasks {
    const uint8_t BYTE_DOORS      = 0;
    const uint8_t MASK_DOOR_DRIVER = 0b00000001; // Bit 0
    const uint8_t MASK_DOOR_PASS   = 0b00000010; // Bit 1
    const uint8_t MASK_BOOT        = 0b00000100; // Bit 2
}