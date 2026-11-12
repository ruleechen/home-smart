#include <Arduino.h>
#include <SPI.h>
#include <LT8920.h>

namespace {

  // ESP-12F HSPI: SCK=GPIO14, MISO=GPIO12, MOSI=GPIO13.
  constexpr uint8_t PIN_RF_CS = 4;   // NodeMCU D2
  constexpr uint8_t PIN_RF_PKT = 5;  // NodeMCU D1, SDK default: active high / rising edge
  constexpr uint8_t PIN_RF_RST = 16; // NodeMCU D0; connect LT8920 RST to GPIO16.
  constexpr uint8_t PKT_ACTIVE_LEVEL = HIGH;

  constexpr uint8_t RF_CHANNEL = 74; // 2402 + 74 = 2476 MHz
  constexpr uint32_t SEND_INTERVAL_MS = 2000;
  constexpr uint32_t TX_TIMEOUT_MS = 100;
  constexpr LT8920::DataRate RF_DATA_RATE = LT8920::LT8920_1MBPS;

  // Assumption: captured high/low byte pairs for registers 36, 37, 38, 39.
  const uint8_t SYNC_WORD[8] = {0x05, 0x0A, 0x03, 0x80, 0x5A, 0x5A, 0x55, 0xAA};

  // Complete FIFO images: 0x13 is the length of the following 19 payload bytes.
  const uint8_t CMD_LIGHT_ON[20] = {0x13, 0xA3, 0x71, 0x4D, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x00, 0x38, 0x00, 0x00, 0x03, 0x37, 0x00, 0x57, 0x01};
  const uint8_t CMD_LIGHT_OFF[20] = {0x13, 0xA3, 0x71, 0x4D, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x00, 0x3C, 0x00, 0x00, 0x03, 0x37, 0x00, 0x57, 0x01};

  constexpr uint8_t REG_CHANNEL = 7;
  constexpr uint8_t REG_SYNC_CONFIG = 32;
  constexpr uint8_t REG_SYNC_WORD = 36;
  constexpr uint8_t REG_FIFO = 50;
  constexpr uint8_t REG_FIFO_CONTROL = 52;
  constexpr uint16_t TX_ENABLE = 0x0100;

  LT8920& radio() {
    // Construct after Arduino startup because the SDK constructor configures GPIO.
    static LT8920 instance(PIN_RF_CS, PIN_RF_PKT, PIN_RF_RST);
    return instance;
  }

  bool radioReady = false;
  bool nextLightOn = true;
  uint32_t lastSendMs = 0;

  bool sendLightCommand(const uint8_t (&fifo)[20]) {
    LT8920& rf = radio();
    rf.writeRegister(REG_CHANNEL, RF_CHANNEL);  // Disable RX/TX.
    rf.writeRegister(REG_FIFO_CONTROL, 0x8000); // Clear TX FIFO/PKT flag.

    // An active flag before TX cannot indicate completion of this packet.
    if (digitalRead(PIN_RF_PKT) == PKT_ACTIVE_LEVEL) {
      Serial.println(F("PKT stayed HIGH after FIFO reset; check wiring/polarity."));
      return false;
    }

    // Do not call sendPacket() with this FIFO image: it adds a length byte.
    for (size_t i = 0; i < sizeof(fifo); i += 2) {
      rf.writeRegister2(REG_FIFO, fifo[i], fifo[i + 1]);
    }
    rf.writeRegister(REG_CHANNEL, RF_CHANNEL | TX_ENABLE);

    const uint32_t startedMs = millis();
    while (digitalRead(PIN_RF_PKT) != PKT_ACTIVE_LEVEL) {
      if (static_cast<uint32_t>(millis() - startedMs) >= TX_TIMEOUT_MS) {
        rf.writeRegister(REG_CHANNEL, RF_CHANNEL);
        rf.writeRegister(REG_FIFO_CONTROL, 0x8000);
        Serial.println(F("TX timeout; check LT8920 connection and PKT pin."));
        return false;
      }
      delay(1); // Service ESP8266 background tasks/watchdog.
    }

    rf.writeRegister(REG_CHANNEL, RF_CHANNEL);
    return true;
  }

} // namespace

void setup(void) {
  Serial.begin(115200);
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setFrequency(1000000);
  delay(100);

  LT8920& rf = radio();
  rf.begin();
  rf.setChannel(RF_CHANNEL);

  // Keep the SDK's preamble/other settings while selecting a 64-bit sync word.
  // Its setSyncWordLength() helper does not preserve the other register bits.
  const uint16_t syncConfig = rf.readRegister(REG_SYNC_CONFIG);
  rf.writeRegister(REG_SYNC_CONFIG, (syncConfig & ~0x1800U) | 0x1800U);
  for (size_t i = 0; i < sizeof(SYNC_WORD); i += 2) {
    rf.writeRegister2(REG_SYNC_WORD + i / 2, SYNC_WORD[i], SYNC_WORD[i + 1]);
  }

  radioReady = rf.setDataRate(RF_DATA_RATE);
  Serial.println(radioReady
    ? F("LT8920 ready: channel 74, alternating lights every 2s.")
    : F("LT8920 register check failed; transmission disabled."));
  lastSendMs = millis();
}

void loop(void) {
  const uint32_t nowMs = millis();
  if (radioReady && static_cast<uint32_t>(nowMs - lastSendMs) >= SEND_INTERVAL_MS) {
    lastSendMs = nowMs;
    const bool sent = sendLightCommand(nextLightOn ? CMD_LIGHT_ON : CMD_LIGHT_OFF);
    Serial.print(nextLightOn ? F("CMD_LIGHT_ON: ") : F("CMD_LIGHT_OFF: "));
    Serial.println(sent ? F("TX complete") : F("TX failed"));
    nextLightOn = !nextLightOn;
  }
  delay(1);
}
