#include <Arduino.h>

#include "rs485.h"
#include "task.h"

String deviceAddr = "";

void startupTask(void* param) {
    Serial.println("startupTask start");
    registerDevice(deviceAddr);
    xTaskCreate(processTask, "process", 4096, NULL, 1, NULL);
    vTaskDelete(NULL);  // delete self when done
}

void setup() {
    initRS485();
    xTaskCreate(startupTask, "startup", 4096, NULL, 1, NULL);
}

void loop() {
}
