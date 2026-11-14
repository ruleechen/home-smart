#include <Arduino.h>
#include <LittleFS.h>
#include <Console.h>
#include <OppleFE6300.h>

using namespace Victor;

namespace {

  // ESP-12F HSPI: SCK=GPIO14, MISO=GPIO12, MOSI=GPIO13. See README for wiring.
  constexpr OppleFE6300::Pins RF_PINS = {
    4,   // CS:  NodeMCU D2
    5,   // PKT: NodeMCU D1
    16,  // RST: NodeMCU D0
  };

  constexpr uint32_t SEND_INTERVAL_MS = 2000;

  // Command names from opple-fe6300.json.
  const String LIGHT_ON = "lightOn";
  const String LIGHT_OFF = "lightOff";

  OppleFE6300 radio(RF_PINS);
  bool nextLightOn = true;
  uint32_t lastSendMs = 0;

} // namespace

void setup(void) {
  console.begin(115200);

  // Mounted here until the app moves to AppMain, which mounts it itself.
  // On ESP8266 a partition that fails to mount is auto-formatted, so a missing
  // filesystem image shows up below as zero commands, not as a mount failure.
  if (!LittleFS.begin()) {
    console.error()
      .bracket(F("fs"))
      .section(F("mount failed"));
  }

  if (radio.begin()) {
    console.log()
      .bracket(F("radio"))
      .section(F("ready"))
      .section(F("channel"), String(radio.channel()))
      .section(F("commands"), String(radio.commandCount()))
      .section(F("interval"), String(SEND_INTERVAL_MS) + F("ms"));
    // Commands live in the filesystem image, not in the firmware.
    if (radio.commandCount() == 0) {
      console.error()
        .bracket(F("radio"))
        .section(F("no commands loaded; upload the filesystem image: pio run -e opple-fe6300 -t uploadfs"));
    }
  } else {
    console.error()
      .bracket(F("radio"))
      .section(F("register check failed; transmission disabled"));
  }

  lastSendMs = millis();
}

void loop(void) {
  const uint32_t nowMs = millis();
  if (radio.isReady() && nowMs - lastSendMs >= SEND_INTERVAL_MS) {
    lastSendMs = nowMs;
    const auto& command = nextLightOn ? LIGHT_ON : LIGHT_OFF;
    const auto result = radio.send(command);
    (result == OppleFE6300::TxResult::Ok ? console.log() : console.error())
      .bracket(F("radio"))
      .section(command, OppleFE6300::describe(result));
    nextLightOn = !nextLightOn;
  }
}
