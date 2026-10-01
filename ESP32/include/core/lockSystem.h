#pragma once

enum class LockAction {
    DO_LOCK,
    DO_UNLOCK,
    DO_NOTHING
};

// Função de lógica pura (testável)
LockAction decideLockAction(bool ownerNearby, bool isCurrentlyLocked);

// Classe com funções que dependem de hardware
class LockSystem {
public:
    static void handleTouchEvent();
    static void enterDeepSleep();
};