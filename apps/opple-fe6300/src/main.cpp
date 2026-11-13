#include <Arduino.h>
#include <LT8920Radio.h>
#include "OppleCommands.h"

namespace {

  // ESP-12F HSPI: SCK=GPIO14, MISO=GPIO12, MOSI=GPIO13. See README for wiring.
  constexpr LT8920Radio::Pins RF_PINS = {
    4,   // CS:  NodeMCU D2
    5,   // PKT: NodeMCU D1
    16,  // RST: NodeMCU D0
  };

  constexpr uint32_t SEND_INTERVAL_MS = 2000;

  LT8920Radio::Config radioConfig() {
    LT8920Radio::Config config;
    config.channel = 74;  // 2402 + 74 = 2476 MHz
    config.dataRate = LT8920::LT8920_1MBPS;
    // Assumption: captured values for registers 36, 37, 38, 39.
    config.syncWordLength = LT8920Radio::SyncWordLength::Bits64;
    config.syncWord[0] = 0x050A;
    config.syncWord[1] = 0x0380;
    config.syncWord[2] = 0x5A5A;
    config.syncWord[3] = 0x55AA;
    return config;
  }

  LT8920Radio radio(RF_PINS);
  bool nextLightOn = true;
  uint32_t lastSendMs = 0;

} // namespace

void setup(void) {
  Serial.begin(115200);

  if (radio.begin(radioConfig())) {
    Serial.printf_P(PSTR("LT8920 ready: channel %u, alternating lights every %ums.\n"), radio.channel(), SEND_INTERVAL_MS);
  } else {
    Serial.println(F("LT8920 register check failed; transmission disabled."));
  }

  lastSendMs = millis();
}

void loop(void) {
  const uint32_t nowMs = millis();
  if (radio.isReady() && nowMs - lastSendMs >= SEND_INTERVAL_MS) {
    lastSendMs = nowMs;
    const auto result = nextLightOn
      ? radio.send(OppleCommands::LIGHT_ON)
      : radio.send(OppleCommands::LIGHT_OFF);
    Serial.print(nextLightOn
      ? F("LIGHT_ON: ")
      : F("LIGHT_OFF: "));
    Serial.println(LT8920Radio::describe(result));
    nextLightOn = !nextLightOn;
  }
}
