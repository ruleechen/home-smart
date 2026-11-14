#include "OppleFE6300.h"
#include <SPI.h>

namespace {

  struct Register {
    uint8_t address;
    uint16_t value;
  };

  constexpr uint32_t SPI_FREQUENCY_HZ = 1000000;

  constexpr uint8_t REG_CHANNEL = 7;
  constexpr uint16_t CHANNEL_MASK = 0x007F;
  constexpr uint16_t TX_ENABLE = 0x0100;

  constexpr uint8_t REG_SYNC_WORD = 36;  // 36..39
  constexpr uint8_t REG_STATUS = 48;
  constexpr uint16_t STATUS_PKT_FLAG = 0x0040;  // Logical flag, independent of the GPIO polarity bit.
  constexpr uint8_t REG_FIFO = 50;
  constexpr uint8_t REG_FIFO_CONTROL = 52;
  constexpr uint16_t FIFO_CLEAR_TX = 0x8000;

  // Register 41 leaves PKT_FIFO_POLARITY clear: PKT goes high when TX completes.
  constexpr uint8_t PKT_ACTIVE_LEVEL = HIGH;
  // PKT is cleared as soon as TX is enabled; a packet takes far longer than this to send.
  constexpr uint32_t PKT_CLEAR_TIMEOUT_US = 1000;
  // With AUTO_ACK a packet takes about 11 ms from TX enable to PKT.
  constexpr uint32_t TX_TIMEOUT_MS = 100;

  // ---- Captured from the original remote's SPI bus (照明1 / 照明2) ----

  constexpr uint8_t CHANNEL = 74;  // 2402 + 74 = 2476 MHz

  // Registers 36..39. Register 32 selects a 32-bit sync word, so only 36 and 39
  // go on air; 37/38 are written as captured anyway.
  constexpr uint16_t SYNC_WORD[4] = {0x050A, 0x0380, 0x5A5A, 0x55AA};

  // Init writes that differ from the SDK's begin(), applied after it.
  constexpr Register REMOTE_REGISTERS[] = {
    {9, 0x4800},   // Power/gain (SDK: 0x4000).
    {11, 0x0008},  // SDK: 0x0000.
    {32, 0x4800},  // 3-byte preamble, 32-bit sync word (SDK: 0x5000, 48-bit).
    {40, 0x4402},  // Sync word error threshold (SDK: 0x4401).
    {41, 0xB800},  // CRC | PACK_LEN | FW_TERM_TX | AUTO_ACK (SDK: 0xB000, no AUTO_ACK).
    {44, 0x0100},  // 1 Mbps (SDK: 0x0101).
    {45, 0x0152},  // Not written by the SDK.
  };

  // The remote repeats each packet about 12 times, one TX start every ~13 ms.
  constexpr uint8_t BURST_COUNT = 12;
  constexpr uint32_t BURST_PERIOD_MS = 13;

} // namespace

OppleFE6300::OppleFE6300(const Pins& pins, const char* commandsPath)
  : _pins(pins), _storage(commandsPath) {}

OppleFE6300::~OppleFE6300() {
  delete _rf;
  _rf = nullptr;
}

bool OppleFE6300::begin() {
  _ready = false;
  _commands = _storage.load();

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
  _rf->setChannel(CHANNEL);

  // setDataRate() reads the register back, which doubles as an SPI check.
  // Register 44 is then overwritten with the remote's exact value below.
  const bool spiOk = _rf->setDataRate(LT8920::LT8920_1MBPS);

  for (const auto& reg : REMOTE_REGISTERS) {
    _rf->writeRegister(reg.address, reg.value);
  }
  for (uint8_t i = 0; i < 4; i++) {
    _rf->writeRegister(REG_SYNC_WORD + i, SYNC_WORD[i]);
  }

  _ready = spiOk && _verifySyncWord();
  return _ready;
}

bool OppleFE6300::isReady() const {
  return _ready;
}

uint8_t OppleFE6300::channel() const {
  return CHANNEL & CHANNEL_MASK;
}

size_t OppleFE6300::commandCount() const {
  return _commands == nullptr ? 0 : _commands->commands.size();
}

OppleFE6300::TxResult OppleFE6300::send(const String& name) {
  if (!_ready) {
    return TxResult::NotReady;
  }
  const auto command = _findCommand(name);
  if (command == nullptr) {
    return TxResult::UnknownCommand;
  }
  const auto& payload = command->payload;
  if (payload.empty() || payload.size() > MAX_PAYLOAD) {
    return TxResult::InvalidPayload;
  }

  auto result = TxResult::Ok;
  for (uint8_t i = 0; i < BURST_COUNT; i++) {
    const uint32_t startedMs = millis();
    result = _sendPacket(payload.data(), payload.size());
    if (result != TxResult::Ok) {
      break;
    }
    while (millis() - startedMs < BURST_PERIOD_MS) {
      delay(1);
    }
  }
  return result;
}

const __FlashStringHelper* OppleFE6300::describe(TxResult result) {
  switch (result) {
    case TxResult::Ok:
      return F("TX complete");
    case TxResult::NotReady:
      return F("radio not ready");
    case TxResult::UnknownCommand:
      return F("unknown command; check opple-fe6300.json");
    case TxResult::InvalidPayload:
      return F("payload empty or longer than 63 bytes");
    case TxResult::PktMismatch:
      return F("PKT pin disagrees with status register; check PKT wiring/polarity");
    case TxResult::PktStuck:
      return F("PKT did not clear when TX started; check PKT wiring/polarity");
    case TxResult::Timeout:
      return F("TX timeout; check LT8920 connection and PKT pin");
  }
  return F("unknown");
}

LT8920* OppleFE6300::sdk() {
  return _rf;
}

const OppleFE6300Command* OppleFE6300::_findCommand(const String& name) const {
  if (_commands == nullptr) {
    return nullptr;
  }
  for (const auto& command : _commands->commands) {
    if (command.name == name) {
      return &command;
    }
  }
  return nullptr;
}

OppleFE6300::TxResult OppleFE6300::_sendPacket(const uint8_t* payload, size_t length) {
  _idle();
  _flushTx();

  // PKT may legitimately still be active here: the chip keeps it set after the
  // previous packet (including the dummy TX in LT8920::begin()) and only clears it
  // when TX/RX is enabled again. Only a GPIO/status mismatch points at wiring.
  if (_isPktActive() != _pktFlagRegister()) {
    return TxResult::PktMismatch;
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

  // Wait for the stale flag to clear, so the next active level belongs to this packet.
  // Tight loop: the TX itself takes hundreds of microseconds, so the low phase cannot be missed.
  const uint32_t enabledUs = micros();
  while (_isPktActive()) {
    if (micros() - enabledUs >= PKT_CLEAR_TIMEOUT_US) {
      _idle();
      _flushTx();
      return TxResult::PktStuck;
    }
  }

  const uint32_t startedMs = millis();
  while (!_isPktActive()) {
    if (millis() - startedMs >= TX_TIMEOUT_MS) {
      _idle();
      _flushTx();
      return TxResult::Timeout;
    }
    delay(1);  // Service ESP8266 background tasks/watchdog.
  }

  _idle();
  return TxResult::Ok;
}

void OppleFE6300::_idle() {
  _rf->writeRegister(REG_CHANNEL, channel());  // Channel only: RX and TX disabled.
}

void OppleFE6300::_flushTx() {
  _rf->writeRegister(REG_FIFO_CONTROL, FIFO_CLEAR_TX);  // Resets the TX pointer; PKT is not cleared here.
}

bool OppleFE6300::_isPktActive() const {
  return digitalRead(_pins.pkt) == PKT_ACTIVE_LEVEL;
}

bool OppleFE6300::_pktFlagRegister() {
  return (_rf->readRegister(REG_STATUS) & STATUS_PKT_FLAG) != 0;
}

bool OppleFE6300::_verifySyncWord() {
  for (uint8_t i = 0; i < 4; i++) {
    if (_rf->readRegister(REG_SYNC_WORD + i) != SYNC_WORD[i]) {
      return false;
    }
  }
  return true;
}
