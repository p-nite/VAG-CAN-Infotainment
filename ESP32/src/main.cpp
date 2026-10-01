#include <Arduino.h>
#include "config.h"
#include "hardwareDrivers/canManager.h"
#include "hardwareDrivers/bleManager.h"
#include "hardwareDrivers/gsmManager.h"

#ifndef UNIT_TEST

#include "core/lockSystem.h"

// Tarefa para testar o TTP223 na bancada
void taskTouch(void *param) {
    pinMode(PIN_TTP223, INPUT);
    bool lastState = LOW;

    while (true) {
        bool currentState = digitalRead(PIN_TTP223);
        if (currentState == HIGH && lastState == LOW) {
            // Detetou um toque (borda de subida)
            LockSystem::handleTouchEvent();
        }
        lastState = currentState;
        vTaskDelay(100 / portTICK_PERIOD_MS); // Lê a cada 100ms
    }
}

void setup() {
    Serial.begin(115200);
    
    // Inicializar os módulos
    initCAN();
    initBLE();
    initGSM();
    
    // Criar as tarefas do FreeRTOS
    xTaskCreate(taskCAN, "CAN_Task", TASK_STACK_CAN, NULL, 3, NULL);
    xTaskCreate(taskBLE, "BLE_Task", TASK_STACK_BLE, NULL, 2, NULL);
    xTaskCreate(taskGSM, "GSM_Task", TASK_STACK_GSM, NULL, 1, NULL);
    xTaskCreate(taskTouch, "Touch_Task", 4096, NULL, 4, NULL); // Prioridade alta para o toque
}

void loop() {
    // Em FreeRTOS no ESP32, o loop() é apenas uma tarefa de prioridade muito baixa (0).
    // Podes deixá-lo vazio ou apagar a task com vTaskDelete(NULL).
    vTaskDelete(NULL);
}

#endif