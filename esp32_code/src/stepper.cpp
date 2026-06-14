#include "stepper.h"

#include "rs485.h"

QueueHandle_t stepperQueue;
AccelStepper stepperForward(AccelStepper::HALF4WIRE, MOTOR1_IN1, MOTOR1_IN2, MOTOR1_IN3, MOTOR1_IN4);
AccelStepper stepperLateral(AccelStepper::HALF4WIRE, MOTOR2_IN1, MOTOR2_IN2, MOTOR2_IN3, MOTOR2_IN4);

void initStepper() {
    stepperQueue = xQueueCreate(10, CMD_LEN);
    stepperForward.setMaxSpeed(400.0);
    stepperLateral.setMaxSpeed(400.0);
    stepperForward.setAcceleration(100.0);
    stepperLateral.setAcceleration(100.0);
    xTaskCreate(stepperTask, "stepper", 4096, NULL, 1, NULL);
}

void moveCard(const String& cmd) {
    if (cmd == "FWD") {
        stepperForward.move(800);
    } else if (cmd == "LFT") {
        stepperLateral.move(-800);
    } else if (cmd == "RGT") {
        stepperLateral.move(800);
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