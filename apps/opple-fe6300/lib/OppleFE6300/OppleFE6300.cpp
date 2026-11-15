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
  constexpr uint16_t RX_ENABLE = 0x0080;

  constexpr uint8_t REG_SYNC_WORD = 36;  // 36..39
  constexpr uint8_t REG_STATUS = 48;
  constexpr uint16_t STATUS_CRC_ERROR = 0x8000;
  constexpr uint16_t STATUS_SYNC_WORD = 0x0080;  // SYNCWORD_RECV, RX only.
  constexpr uint16_t STATUS_PKT_FLAG = 0x0040;  // Logical flag, independent of the GPIO polarity bit.
  constexpr uint8_t REG_FIFO = 50;
  constexpr uint8_t REG_FIFO_CONTROL = 52;
  // Clears both FIFO pointers. The remote writes this before every packet; clearing
  // only one side lets the pointers drift apart once TX and RX share the FIFO.
  constexpr uint16_t FIFO_CLEAR_BOTH = 0x8080;

  constexpr uint8_t REG_PACKET_CONFIG = 41;
  constexpr uint16_t PACKET_CONFIG_AUTO_ACK = 0x0800;

  // Register 41 leaves PKT_FIFO_POLARITY clear: PKT goes high when TX completes
  // or a packet has been received.
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

  // CRC | PACK_LEN | FW_TERM_TX | AUTO_ACK (SDK: 0xB000, no AUTO_ACK).
  constexpr uint16_t REMOTE_PACKET_CONFIG = 0xB800;

  // Init writes that differ from the SDK's begin(), applied after it.
  constexpr Register REMOTE_REGISTERS[] = {
    {9, 0x4800},   // Power/gain (SDK: 0x4000).
    {11, 0x0008},  // SDK: 0x0000.
    {32, 0x4800},  // 3-byte preamble, 32-bit sync word (SDK: 0x5000, 48-bit).
    {40, 0x4402},  // Sync word error threshold (SDK: 0x4401).
    {REG_PACKET_CONFIG, REMOTE_PACKET_CONFIG},
    {44, 0x0100},  // 1 Mbps (SDK: 0x0101).
    {45, 0x0152},  // Not written by the SDK.
  };

  // The remote repeats each packet about 12 times, one TX start every ~13 ms.
  constexpr uint8_t BURST_COUNT = 12;
  constexpr uint32_t BURST_PERIOD_MS = 13;

  // Payload fields (see OppleFE6300.h).
  constexpr size_t ADDRESS_INDEX = 0;  // 4 bytes.
  constexpr size_t SEQUENCE_INDEX = 10;  // 2 bytes, big-endian.

  // Re-arm RX when no packet has come in for this long. The chip has been seen
  // going deaf; a burst lasts ~170 ms with a packet every ~3 ms, so a quick
  // re-arm still catches most of a burst the chip stopped hearing, while a
  // re-arm that cuts into a packet costs one of ~50 copies.
  constexpr uint32_t RX_REARM_MS = 50;

  // RSSI sampling while idle, for the trace.
  constexpr uint32_t RSSI_SAMPLE_US = 1000;
  // Re-arms merged into one trace event (~1 s), so a long quiet keeps its timing.
  constexpr uint16_t MAX_REARM_RUN = 20;

  uint16_t readSequence(const uint8_t* payload) {
    return (payload[SEQUENCE_INDEX] << 8) | payload[SEQUENCE_INDEX + 1];
  }

  void writeSequence(uint8_t* payload, uint16_t sequence) {
    payload[SEQUENCE_INDEX] = sequence >> 8;
    payload[SEQUENCE_INDEX + 1] = sequence & 0xFF;
  }

  uint32_t readAddress(const uint8_t* payload) {
    return (static_cast<uint32_t>(payload[ADDRESS_INDEX]) << 24) |
      (static_cast<uint32_t>(payload[ADDRESS_INDEX + 1]) << 16) |
      (static_cast<uint32_t>(payload[ADDRESS_INDEX + 2]) << 8) |
      payload[ADDRESS_INDEX + 3];
  }

  bool isField(size_t i) {
    return (i >= ADDRESS_INDEX && i < ADDRESS_INDEX + 4) ||
      (i >= SEQUENCE_INDEX && i < SEQUENCE_INDEX + 2);
  }

  // A burst lasts ~160 ms. The same payload again within this window, measured
  // from the previous repeat, is the same press (a held key keeps extending it).
  constexpr uint32_t REPEAT_WINDOW_MS = 1000;

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
  _ownAddress = 0;
  if (_commands != nullptr && !_commands->commands.empty() &&
      _commands->commands.front().payload.size() > SEQUENCE_INDEX + 1) {
    _ownAddress = readAddress(_commands->commands.front().payload.data());
  }

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
  if (_ready) {
    _listen();
  }
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

  uint8_t packet[MAX_PAYLOAD];
  memcpy(packet, payload.data(), payload.size());
  if (payload.size() > SEQUENCE_INDEX + 1) {
    writeSequence(packet, _nextSequence(readSequence(payload.data())));
  }

  // The remote sends with AUTO_ACK; listening runs without it (see _listen()).
  _idle();
  _rf->writeRegister(REG_PACKET_CONFIG, REMOTE_PACKET_CONFIG);

  auto result = TxResult::Ok;
  for (uint8_t i = 0; i < BURST_COUNT; i++) {
    const uint32_t startedMs = millis();
    result = _sendPacket(packet, payload.size());
    if (result != TxResult::Ok) {
      break;
    }
    while (millis() - startedMs < BURST_PERIOD_MS) {
      delay(1);
    }
  }
  _listen();
  return result;
}

bool OppleFE6300::receive(RxPacket& packet) {
  if (!_ready) {
    return false;
  }
  if (!_isPktActive()) {
    const uint32_t nowUs = micros();
    if (nowUs - _rssiSampledUs >= RSSI_SAMPLE_US) {
      _rssiSampledUs = nowUs;
      _idleRssiMax = max(_idleRssiMax, _rf->getRSSI());
    }
    if (millis() - _armedMs >= RX_REARM_MS) {
      const bool stuck = (_rf->readRegister(REG_STATUS) & STATUS_SYNC_WORD) != 0;
      _record(TraceEvent::Kind::Rearm, millis(), 0, stuck);
      _listen();
    }
    return false;
  }

  // Each read of the FIFO register pops two bytes; the first byte is the length
  // (PACK_LEN_ENABLE), mirroring the TX layout in _sendPacket().
  bool valid = false;
  if ((_rf->readRegister(REG_STATUS) & STATUS_CRC_ERROR) != 0) {
    _rxStats.crcErrors++;
    _record(TraceEvent::Kind::CrcError, millis());
  } else {
    uint16_t word = _rf->readRegister(REG_FIFO);
    const uint8_t length = word >> 8;
    if (length == 0) {
      _rxStats.empty++;
      _record(TraceEvent::Kind::Empty, millis());
    } else if (length > MAX_PAYLOAD) {
      _rxStats.oversize++;
      _record(TraceEvent::Kind::Oversize, millis());
    } else {
      packet.length = length;
      packet.payload[0] = word & 0xFF;
      for (size_t i = 1; i < length; i += 2) {
        word = _rf->readRegister(REG_FIFO);
        packet.payload[i] = word >> 8;
        if (i + 1 < length) {
          packet.payload[i + 1] = word & 0xFF;
        }
      }
      valid = true;
    }
  }
  _listen();  // The chip stops after one packet; re-arm for the next repeat.

  if (!valid) {
    return false;
  }
  _rxStats.packets++;
  _record(TraceEvent::Kind::Packets, millis(), packet.sequence());

  const uint32_t nowMs = millis();
  const bool repeat = packet.length == _lastRx.length &&
    memcmp(packet.payload, _lastRx.payload, packet.length) == 0 &&
    nowMs - _lastRxMs < REPEAT_WINDOW_MS;
  _lastRxMs = nowMs;
  if (repeat) {
    _rxStats.lastCopies++;
    return false;
  }
  packet.previousCopies = _rxStats.lastCopies;
  _rxStats.lastCopies = 1;

  packet.command = _matchCommand(packet.payload, packet.length);
  packet.own = false;
  packet.missed = 0;
  // Unknown keys count too: they use up a counter value like any other press,
  // and skipping them would show the next known press as a missed one.
  if (packet.length > SEQUENCE_INDEX + 1) {
    packet.missed = _trackRemote(packet.address(), packet.sequence());
    packet.own = packet.address() == _ownAddress;
    if (packet.own) {
      _remoteSequence = packet.sequence();
      _hasRemoteSequence = true;
      _airSequence = _remoteSequence;
      _hasAirSequence = true;
    }
  }
  _lastRx = packet;
  return true;
}

// Records the remote's counter and returns how many presses it skipped. The
// counter moves forward by one per press; a step back or a jump of half the
// range or more is not counted as missed.
uint16_t OppleFE6300::_trackRemote(uint32_t address, uint16_t sequence) {
  for (size_t i = 0; i < _remoteCount; i++) {
    auto& remote = _remotes[i];
    if (remote.address == address) {
      const uint16_t step = sequence - remote.sequence;
      remote.sequence = sequence;
      return (step > 1 && step < 0x8000) ? step - 1 : 0;
    }
  }
  // New remote: take a free slot, or recycle the oldest one.
  const size_t slot = _remoteCount < MAX_REMOTES ? _remoteCount++ : _nextRemoteSlot;
  _nextRemoteSlot = (slot + 1) % MAX_REMOTES;
  _remotes[slot] = {address, sequence};
  return 0;
}

const OppleFE6300::RxStats& OppleFE6300::rxStats() const {
  return _rxStats;
}

size_t OppleFE6300::traceCount() const {
  return _traceCount;
}

const OppleFE6300::TraceEvent& OppleFE6300::traceAt(size_t index) const {
  return _trace[(_traceStart + index) % TRACE_SIZE];
}

// Extends the latest event when it is a run of the same kind (and press
// counter), otherwise appends one, dropping the oldest when full.
void OppleFE6300::_record(TraceEvent::Kind kind, uint32_t ms, uint16_t sequence, bool stuck) {
  const uint8_t rssi = _idleRssiMax;
  _idleRssiMax = 0;
  if (_traceCount > 0) {
    auto& last = _trace[(_traceStart + _traceCount - 1) % TRACE_SIZE];
    const bool sameRun = last.kind == kind &&
      (kind != TraceEvent::Kind::Packets || last.sequence == sequence) &&
      (kind != TraceEvent::Kind::Rearm || last.count < MAX_REARM_RUN);
    if (sameRun) {
      last.count++;
      last.stuck += stuck ? 1 : 0;
      last.rssiMax = max(last.rssiMax, rssi);
      last.lastMs = ms;
      return;
    }
  }
  const TraceEvent event = {kind, 1, sequence, static_cast<uint16_t>(stuck ? 1 : 0), rssi, ms, ms};
  if (_traceCount < TRACE_SIZE) {
    _trace[(_traceStart + _traceCount++) % TRACE_SIZE] = event;
  } else {
    _trace[_traceStart] = event;
    _traceStart = (_traceStart + 1) % TRACE_SIZE;
  }
}

OppleFE6300::Diagnostics OppleFE6300::diagnostics() {
  Diagnostics result = {};
  result.ready = _ready;
  result.pktPin = _isPktActive();
  result.rx = _rxStats;
  result.hasTxSequence = _hasAirSequence;
  result.txSequence = _airSequence;
  if (_rf != nullptr) {
    result.channelRegister = _rf->readRegister(REG_CHANNEL);
    result.packetConfig = _rf->readRegister(REG_PACKET_CONFIG);
    result.status = _rf->readRegister(REG_STATUS);
    result.fifoControl = _rf->readRegister(REG_FIFO_CONTROL);
    result.rssi = _rf->getRSSI();
  }
  return result;
}

OppleFE6300::WatchResult OppleFE6300::watch(uint32_t durationMs) {
  WatchResult result = {};
  result.rssiMin = 0xFF;
  if (!_ready) {
    result.rssiMin = 0;
    return result;
  }
  _listen();
  bool syncBefore = false;
  const uint32_t startedMs = millis();
  while (millis() - startedMs < durationMs) {
    const uint16_t status = _rf->readRegister(REG_STATUS);
    const uint8_t rssi = _rf->getRSSI();
    result.samples++;
    result.rssiMin = min(result.rssiMin, rssi);
    result.rssiMax = max(result.rssiMax, rssi);
    const bool sync = (status & STATUS_SYNC_WORD) != 0;
    if (sync && !syncBefore) {
      result.syncWords++;
    }
    syncBefore = sync;
    if ((status & STATUS_PKT_FLAG) != 0) {
      result.packets++;
      if ((status & STATUS_CRC_ERROR) != 0) {
        result.crcErrors++;
      }
      if (_isPktActive()) {
        result.pktPin++;
      }
      _listen();
      syncBefore = false;
    }
    yield();
  }
  _listen();
  return result;
}

void OppleFE6300::scan(uint32_t durationMs, uint8_t maxRssi[CHANNEL_COUNT]) {
  memset(maxRssi, 0, CHANNEL_COUNT);
  if (!_ready) {
    return;
  }
  const uint32_t startedMs = millis();
  while (millis() - startedMs < durationMs) {
    for (uint8_t ch = 0; ch < CHANNEL_COUNT; ch++) {
      _rf->writeRegister(REG_CHANNEL, ch);  // Idle, so enabling RX recalibrates the VCO.
      _rf->writeRegister(REG_CHANNEL, ch | RX_ENABLE);
      delayMicroseconds(300);
      maxRssi[ch] = max(maxRssi[ch], _rf->getRSSI());
    }
    yield();
  }
  _listen();
}

uint16_t OppleFE6300::RxPacket::sequence() const {
  return length > SEQUENCE_INDEX + 1 ? readSequence(payload) : 0;
}

uint32_t OppleFE6300::RxPacket::address() const {
  return length >= ADDRESS_INDEX + 4 ? readAddress(payload) : 0;
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

// A stored command matches when every byte but the address and the press
// counter is equal.
const OppleFE6300Command* OppleFE6300::_matchCommand(const uint8_t* payload, size_t length) const {
  if (_commands == nullptr) {
    return nullptr;
  }
  for (const auto& command : _commands->commands) {
    if (command.payload.size() != length) {
      continue;
    }
    bool equal = true;
    for (size_t i = 0; i < length && equal; i++) {
      equal = isField(i) || command.payload[i] == payload[i];
    }
    if (equal) {
      return &command;
    }
  }
  return nullptr;
}

// The heater drops a packet whose counter equals the previous press (that is
// how it collapses the repeats of one press), so a counter must differ from
// the last one on air. Counting down from the remote's latest value keeps clear
// of the counters the remote will use next (+1, +2, ...), and alternating
// between -1 and -2 keeps consecutive sends from this side distinct.
// Until the remote has been heard, the stored command's counter stands in.
uint16_t OppleFE6300::_nextSequence(uint16_t templateSequence) {
  const uint16_t anchor = _hasRemoteSequence ? _remoteSequence : static_cast<uint16_t>(templateSequence + 1);
  uint16_t sequence = anchor - 1;
  if (_hasAirSequence && sequence == _airSequence) {
    sequence = anchor - 2;
  }
  _airSequence = sequence;
  _hasAirSequence = true;
  return sequence;
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

void OppleFE6300::setRxAutoAck(bool enabled) {
  _rxAutoAck = enabled;
  if (_ready) {
    _listen();
  }
}

bool OppleFE6300::rxAutoAck() const {
  return _rxAutoAck;
}

// Passive listening by default: AUTO_ACK is cleared so this side never answers
// the remote's packets on air. send() restores the remote's configuration.
void OppleFE6300::_listen() {
  _idle();
  _rf->writeRegister(REG_PACKET_CONFIG,
    _rxAutoAck ? REMOTE_PACKET_CONFIG : (REMOTE_PACKET_CONFIG & ~PACKET_CONFIG_AUTO_ACK));
  _rf->writeRegister(REG_FIFO_CONTROL, FIFO_CLEAR_BOTH);
  _rf->writeRegister(REG_CHANNEL, channel() | RX_ENABLE);  // Also clears PKT.
  _armedMs = millis();
}

void OppleFE6300::_idle() {
  _rf->writeRegister(REG_CHANNEL, channel());  // Channel only: RX and TX disabled.
}

void OppleFE6300::_flushTx() {
  _rf->writeRegister(REG_FIFO_CONTROL, FIFO_CLEAR_BOTH);  // PKT is not cleared here.
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
