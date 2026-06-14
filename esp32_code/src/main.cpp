#include <Arduino.h>

#include "rs485.h"
#include "stepper.h"
#include "task.h"

String deviceAddr = "";

void startupTask(void* param) {
    Serial.println("startupTask start");
    registerDevice(deviceAddr);
    xTaskCreate(processTask, "process", 4096, NULL, 1, NULL);
    vTaskDelete(NULL);  // delete self when done
}

void setup() {
    // testing stepper stuff
    // Serial.begin(115200);
    // delay(100);  // let hardware settle
    // AccelStepper test(AccelStepper::FULL4WIRE, 25, 27, 26, 14);
    // test.setMaxSpeed(50.0);
    // test.setAcceleration(20.0);
    // test.move(2048);
    // while (test.isRunning()) {
    //     test.run();
    // }
    // Serial.println("done");

    initRS485();
    initStepper();
    xTaskCreate(startupTask, "startup", 4096, NULL, 1, NULL);
}

void loop() {
}
