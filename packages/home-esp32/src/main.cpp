#include <Arduino.h>
#include "DoorSensor.h"

using namespace Victor;
using namespace Victor::Components;

bool ledOn = false;
void turnLedOn() {
  digitalWrite(LED_BUILTIN, HIGH);
  ledOn = true;
}
void turnLedOff() {
  digitalWrite(LED_BUILTIN, LOW);
  ledOn = false;
}
void toggleLed() {
  if (ledOn) {
    turnLedOff();
  } else {
    turnLedOn();
  }
}
void flashLed(unsigned long ms = 100) {
  toggleLed();
  delay(ms); // at least light for some time
  toggleLed();
}

DoorSensor* accessory = nullptr;
static String message = "";

void setup(void) {
  Serial.begin(115200);
  // led
  pinMode(BUILTIN_LED, OUTPUT);
  turnLedOn();
  // acc
  accessory = new DoorSensor();
  accessory->setup();
  // done
  Serial.println();
  Serial.println("setup complete");
  turnLedOff();
}

void loop(void) {
  accessory->loop();

  auto isEnterPressed = false;
  while (Serial.available()) {
    const auto ch = Serial.read();
    isEnterPressed = (ch == '\r');
    if (!isEnterPressed) {
      message += (char)ch;
    } else {
      break;
    }
  }

  if (isEnterPressed) {
    message.trim();
    if (message == "open") {
      accessory->setOpen();
    } else if (message == "close") {
      accessory->setClosed();
    }
    flashLed();
    message = "";
    return;
  }
}
