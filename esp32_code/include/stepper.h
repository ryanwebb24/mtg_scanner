#ifndef STEPPER_H
#define STEPPER_H

#include <AccelStepper.h>
#include <Arduino.h>

// Motor 1
#define MOTOR1_IN1 25
#define MOTOR1_IN2 26
#define MOTOR1_IN3 27
#define MOTOR1_IN4 14

// Motor 2
#define MOTOR2_IN1 12
#define MOTOR2_IN2 13
#define MOTOR2_IN3 32
#define MOTOR2_IN4 33

extern QueueHandle_t stepperQueue;

void initStepper();
void moveCard(const String& cmd);
void stepperTask(void* param);

#endif