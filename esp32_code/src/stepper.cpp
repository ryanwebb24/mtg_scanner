#include "stepper.h"

#include "rs485.h"

QueueHandle_t stepperQueue;
AccelStepper stepperForward(AccelStepper::FULL4WIRE, MOTOR1_IN1, MOTOR1_IN2, MOTOR1_IN3, MOTOR1_IN4);
AccelStepper stepperLateral(AccelStepper::FULL4WIRE, MOTOR2_IN1, MOTOR2_IN2, MOTOR2_IN3, MOTOR2_IN4);

void initStepper() {
    stepperQueue = xQueueCreate(10, CMD_LEN);
    stepperForward.setMaxSpeed(800.0);
    stepperLateral.setMaxSpeed(800.0);
    stepperForward.setAcceleration(200.0);
    stepperLateral.setAcceleration(200.0);
    xTaskCreate(stepperTask, "stepper", 4096, NULL, 1, NULL);
}

void moveCard(const String& cmd) {
    if (cmd == "FWD") {
        stepperForward.move(4096);
    } else if (cmd == "LFT") {
        stepperLateral.move(-4096);
    } else if (cmd == "RGT") {
        stepperLateral.move(4096);
    }
}

void stepperTask(void* param) {
    char cmd[CMD_LEN];
    while (true) {
        if (xQueueReceive(stepperQueue, cmd, 0)) {
            moveCard(String(cmd));
        }
        stepperForward.run();
        stepperLateral.run();
        if (!stepperForward.isRunning() && !stepperLateral.isRunning()) {
            vTaskDelay(pdMS_TO_TICKS(10));
        } else {
            vTaskDelay(pdMS_TO_TICKS(1));  // yield even when running
        }
    }
}