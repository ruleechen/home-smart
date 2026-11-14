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

  // The remote repeats each packet about 12 times, one TX start every ~13 ms.
  constexpr uint8_t BURST_COUNT = 12;
  constexpr uint32_t BURST_PERIOD_MS = 13;

  // Init writes from the remote's SPI capture that differ from the SDK's begin().
  // Register 32 (0x4800) comes from syncWordLength = Bits32 below.
  constexpr LT8920Radio::Register REMOTE_REGISTERS[] = {
    {9, 0x4800},   // Power/gain (SDK: 0x4000).
    {11, 0x0008},  // SDK: 0x0000.
    {40, 0x4402},  // Sync word error threshold (SDK: 0x4401).
    {41, 0xB800},  // CRC | PACK_LEN | FW_TERM_TX | AUTO_ACK (SDK: 0xB000, no AUTO_ACK).
    {44, 0x0100},  // 1 Mbps (SDK: 0x0101).
    {45, 0x0152},  // Not written by the SDK.
  };

  LT8920Radio::Config radioConfig() {
    LT8920Radio::Config config;
    config.channel = 74;  // 2402 + 74 = 2476 MHz
    config.dataRate = LT8920::LT8920_1MBPS;
    // Captured: register 32 = 0x4800 -> 3-byte preamble, 32-bit sync word,
    // which uses registers 36 and 39 only. 37/38 are written as captured anyway.
    config.syncWordLength = LT8920Radio::SyncWordLength::Bits32;
    config.syncWord[0] = 0x050A;
    config.syncWord[1] = 0x0380;
    config.syncWord[2] = 0x5A5A;
    config.syncWord[3] = 0x55AA;
    config.registers = REMOTE_REGISTERS;
    config.registerCount = sizeof(REMOTE_REGISTERS) / sizeof(REMOTE_REGISTERS[0]);
    return config;
  }

  LT8920Radio radio(RF_PINS);

  // Returns the first failure, or the last result when every packet went out.
  template <size_t N>
  LT8920Radio::TxResult sendBurst(const uint8_t (&payload)[N]) {
    auto result = LT8920Radio::TxResult::Ok;
    for (uint8_t i = 0; i < BURST_COUNT; i++) {
      const uint32_t startedMs = millis();
      result = radio.send(payload);
      if (result != LT8920Radio::TxResult::Ok) {
        break;
      }
      while (millis() - startedMs < BURST_PERIOD_MS) {
        delay(1);
      }
    }
    return result;
  }

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
      ? sendBurst(OppleCommands::LIGHT_ON)
      : sendBurst(OppleCommands::LIGHT_OFF);
    Serial.print(nextLightOn
      ? F("LIGHT_ON: ")
      : F("LIGHT_OFF: "));
    Serial.println(LT8920Radio::describe(result));
    nextLightOn = !nextLightOn;
  }
}
