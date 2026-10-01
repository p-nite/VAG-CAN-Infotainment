#pragma once
#include <stdint.h>

namespace AuthService {
    void init();
    bool isOwnerNearby();

    bool isMAConWhitelist(const char* foundMAC);
}