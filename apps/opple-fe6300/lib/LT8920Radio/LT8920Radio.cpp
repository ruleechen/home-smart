#include "LT8920Radio.h"
#include <SPI.h>

namespace {

  constexpr uint32_t SPI_FREQUENCY_HZ = 1000000;

  constexpr uint8_t REG_CHANNEL = 7;
  constexpr uint16_t CHANNEL_MASK = 0x007F;
  constexpr uint16_t TX_ENABLE = 0x0100;

  constexpr uint8_t REG_SYNC_CONFIG = 32;
  constexpr uint8_t SYNC_LENGTH_SHIFT = 11;
  constexpr uint16_t SYNC_LENGTH_MASK = 0x03 << SYNC_LENGTH_SHIFT;

  constexpr uint8_t REG_SYNC_WORD = 36;  // 36..39
  constexpr uint8_t REG_FIFO = 50;
  constexpr uint8_t REG_FIFO_CONTROL = 52;
  constexpr uint16_t FIFO_CLEAR_TX = 0x8000;

} // namespace

LT8920Radio::LT8920Radio(const Pins& pins) : _pins(pins) {}

LT8920Radio::~LT8920Radio() {
  delete _rf;
  _rf = nullptr;
}

bool LT8920Radio::begin(const Config& config) {
  _config = config;
  _ready = false;

  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setFrequency(SPI_FREQUENCY_HZ);
  delay(100);

  // Created here rather than at static-init time: the SDK constructor configures GPIO.
  if (_rf == nullptr) {
    _rf = new LT8920(_pins.cs, _pins.pkt, _pins.rst);
  }
  _rf->begin();
  _rf->setChannel(_config.channel);
  _writeSyncWord();

  // setDataRate() reads the register back, which doubles as an SPI check.
  _ready = _rf->setDataRate(_config.dataRate) && _verifySyncWord();
  return _ready;
}

bool LT8920Radio::isReady() const {
  return _ready;
}

uint8_t LT8920Radio::channel() const {
  return _config.channel & CHANNEL_MASK;
}

LT8920Radio::TxResult LT8920Radio::send(const uint8_t* payload, size_t length) {
  if (!_ready) {
    return TxResult::NotReady;
  }
  if (payload == nullptr || length == 0 || length > MAX_PAYLOAD) {
    return TxResult::InvalidLength;
  }

  _idle();
  _flushTx();

  // An active flag before TX cannot indicate completion of this packet.
  if (_isPktActive()) {
    return TxResult::PktStuck;
  }

  // The FIFO takes 16-bit words: [length, payload[0]], [payload[1], payload[2]], ...
  // Same layout as LT8920::sendPacket(), which is not used because it reads past
  // the buffer for even lengths and waits on PKT without a timeout.
  _rf->writeRegister2(REG_FIFO, static_cast<uint8_t>(length), payload[0]);
  for (size_t i = 1; i < length; i += 2) {
    const uint8_t low = (i + 1 < length) ? payload[i + 1] : 0x00;  // Pad beyond the length byte.
    _rf->writeRegister2(REG_FIFO, payload[i], low);
  }
  _rf->writeRegister(REG_CHANNEL, channel() | TX_ENABLE);

  const uint32_t startedMs = millis();
  while (!_isPktActive()) {
    if (millis() - startedMs >= _config.txTimeoutMs) {
      _idle();
      _flushTx();
      return TxResult::Timeout;
    }
    delay(1);  // Service ESP8266 background tasks/watchdog.
  }

  _idle();
  return TxResult::Ok;
}

const __FlashStringHelper* LT8920Radio::describe(TxResult result) {
  switch (result) {
    case TxResult::Ok:
      return F("TX complete");
    case TxResult::NotReady:
      return F("radio not ready");
    case TxResult::InvalidLength:
      return F("invalid payload length");
    case TxResult::PktStuck:
      return F("PKT stayed active after FIFO reset; check wiring/polarity");
    case TxResult::Timeout:
      return F("TX timeout; check LT8920 connection and PKT pin");
  }
  return F("unknown");
}

LT8920* LT8920Radio::sdk() {
  return _rf;
}

void LT8920Radio::_idle() {
  _rf->writeRegister(REG_CHANNEL, channel());  // Channel only: RX and TX disabled.
}

void LT8920Radio::_flushTx() {
  _rf->writeRegister(REG_FIFO_CONTROL, FIFO_CLEAR_TX);  // Also clears the PKT flag.
}

bool LT8920Radio::_isPktActive() const {
  return digitalRead(_pins.pkt) == _config.pktActiveLevel;
}

void LT8920Radio::_writeSyncWord() {
  // Keep the SDK's preamble/trailer settings; only replace the sync word length bits.
  // The SDK's setSyncWordLength() keeps only the length bits and drops the rest.
  const uint16_t syncConfig = _rf->readRegister(REG_SYNC_CONFIG);
  const uint16_t lengthBits = static_cast<uint16_t>(_config.syncWordLength) << SYNC_LENGTH_SHIFT;
  _rf->writeRegister(REG_SYNC_CONFIG, (syncConfig & ~SYNC_LENGTH_MASK) | lengthBits);

  for (uint8_t i = 0; i < 4; i++) {
    _rf->writeRegister(REG_SYNC_WORD + i, _config.syncWord[i]);
  }
}

bool LT8920Radio::_verifySyncWord() {
  for (uint8_t i = 0; i < 4; i++) {
    if (_rf->readRegister(REG_SYNC_WORD + i) != _config.syncWord[i]) {
      return false;
    }
  }
  return true;
}
