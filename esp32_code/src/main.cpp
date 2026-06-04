#include <Arduino.h>

#include "rs485.h"
#include "tasks.h"

String deviceAddr = "";

void setup() {
    initRS485();
    registerDevice(deviceAddr);
    xTaskCreate(processTask, "process", 4096, NULL, 1, NULL);
}

void loop() {
}
