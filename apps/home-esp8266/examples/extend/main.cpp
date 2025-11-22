#include <Arduino.h>

#include "Console.h"
#include "BuiltinLed.h"
#include "OTA/VictorOTA.h"
#include "WiFi/VictorWiFi.h"
#include "WebPortal.h"

using namespace Victor;
using namespace Victor::Components;

WebPortal webPortal(80);

void setup(void) {
  console.begin(115200);
  if (!LittleFS.begin()) {
    console.error()
      .bracket(F("fs"))
      .section(F("mount failed"));
  }

  builtinLed.setup("/led.json");
  builtinLed.turnOn();

  victorOTA.setup("/ota.json");
  victorWiFi.setup("/wifi.json");

  webPortal.onRequestStart = []() { builtinLed.toggle(); };
  webPortal.onRequestEnd = []() { builtinLed.toggle(); };
  webPortal.setup();

  console.log()
    .bracket(F("setup"))
    .section(F("complete"));
}

void loop(void) {
  webPortal.loop();
}
