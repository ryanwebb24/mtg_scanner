#include <Stepper.h>

const int stepsPerRevolution = 2048; // 28BYJ-48
Stepper myStepper(stepsPerRevolution, 16, 18, 17, 19);

const int buttonPin = 4;
int lastButtonState = HIGH;

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  myStepper.setSpeed(10); // max safe speed for 28BYJ-48
}

void loop() {
  int buttonState = digitalRead(buttonPin);

  // On button press, rotate 2 full revolutions (720°)
  if (lastButtonState == HIGH && buttonState == LOW) {
    myStepper.step(stepsPerRevolution * 2); // 2 full revs
  }

  lastButtonState = buttonState;
}
