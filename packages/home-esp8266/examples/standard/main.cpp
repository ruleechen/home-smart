#include <Arduino.h>

#include "Console.h"
#include "BuiltinLed.h"
#include "VictorOTA.h"
#include "VictorWifi.h"
#include "VictorRadio.h"
#include "VictorWeb.h"

using namespace Victor;
using namespace Victor::Components;

VictorRadio radioPortal;
VictorWeb webPortal(80);

void setup(void) {
  console.begin(115200);
  if (!LittleFS.begin()) {
    console.error()
      .bracket(F("fs"))
      .section(F("mount failed"));
  }

  const auto appSetting = appStorage.load();
  builtinLed.setup(appSetting->led);
  builtinLed.turnOn();
  victorOTA.setup("/ota.json");
  victorWifi.setup("/wifi.json");

  radioPortal.onEmit = [](const RadioEmit* emit) {
    builtinLed.flash();
    // emit via your radio tool
    console.log()
      .bracket(F("radio"))
      .section(F("sent"), emit->value)
      .section(F("via channel"), String(emit->channel));
  };

  webPortal.onRequestStart = []() { builtinLed.toggle(); };
  webPortal.onRequestEnd = []() { builtinLed.toggle(); };
  webPortal.onRadioEmit = [](uint8_t index) { radioPortal.emit(index); };
  webPortal.setup();

  console.log()
    .bracket(F("setup"))
    .section(F("complete"));
}

void loop(void) {
  webPortal.loop();
  // receive from your radio tool
  if (false) {
    auto value = String(F(""));
    uint8_t channel = 1;
    radioPortal.receive(value, channel);
    builtinLed.flash();
    console.log()
      .bracket(F("radio"))
      .section(F("received"), value)
      .section(F("from channel"), String(channel));
  }
}
