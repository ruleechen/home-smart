#include <Arduino.h>

bool ledOn = false;
void turnLedOn() {
  digitalWrite(LED_BUILTIN, HIGH);
  ledOn = true;
}
void turnLedOff() {
  digitalWrite(LED_BUILTIN, LOW);
  ledOn = false;
}

void setup(void) {
  Serial.begin(115200);
  // led
  pinMode(BUILTIN_LED, OUTPUT);
  turnLedOn();
}

void loop(void) {
}
