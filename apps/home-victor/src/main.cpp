#include <Arduino.h>

#include <LittleFS.h>

#include <Console/Console.h>
#include <VictorFS/VictorFS.h>

using namespace Victor;

void setup(void) {
  console.begin(115200);

  // mount filesystem (ESP8266 / ESP32 both support LittleFS)
  if (!LittleFS.begin()) {
    console.error()
      .bracket(F("fs"))
      .section(F("mount failed"));
    return;
  }

  // bind to the generic filesystem abstraction layer
  victorFS.begin(LittleFS);

  const char* testPath = "/hello.txt";
  if (victorFS.exists(testPath)) {
    console.log()
      .bracket(F("fs"))
      .section(F("exists"), testPath);
  }

  console.log()
    .bracket(F("setup"))
    .section(F("complete"));
}

void loop(void) {
  delay(1000);
}
