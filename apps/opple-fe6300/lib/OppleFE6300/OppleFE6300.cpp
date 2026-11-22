#include "OppleFE6300.h"
#include <SPI.h>

namespace {

  struct Register {
    uint8_t address;
    uint16_t value;
  };

  constexpr uint32_t SPI_FREQUENCY_HZ = 4000000;

  constexpr uint8_t REG_CHANNEL = 7;
  constexpr uint8_t REG_CURRENT_CONTROL = 9;
  constexpr uint8_t REG_RSSI_CONTROL = 11;
  constexpr uint8_t REG_PACKET_FORMAT = 32;
  constexpr uint8_t REG_SYNC_WORD = 36;  // 36..39
  constexpr uint8_t REG_SYNC_THRESHOLD = 40;
  constexpr uint8_t REG_PACKET_CONFIG = 41;
  constexpr uint8_t REG_DATA_RATE = 44;
  constexpr uint8_t REG_DATA_RATE_OPTIONS = 45;
  constexpr uint8_t REG_STATUS = 48;
  constexpr uint8_t REG_FIFO = 50;
  constexpr uint8_t REG_FIFO_CONTROL = 52;

  constexpr uint16_t CHANNEL_MASK = 0x007F;
  constexpr uint16_t TX_ENABLE = 0x0100;
  constexpr uint16_t RX_ENABLE = 0x0080;

  constexpr uint16_t STATUS_CRC_ERROR = 0x8000;
  constexpr uint16_t STATUS_SYNC_WORD = 0x0080;  // SYNCWORD_RECV, RX only.
  constexpr uint16_t STATUS_PKT_FLAG = 0x0040;  // Logical flag, independent of the GPIO polarity bit.

  constexpr uint16_t PACKET_CONFIG_CRC_ON = 0x8000;
  constexpr uint16_t PACKET_CONFIG_PACK_LEN_ENABLE = 0x2000;
  constexpr uint16_t PACKET_CONFIG_FW_TERM_TX = 0x1000;
  constexpr uint16_t PACKET_CONFIG_AUTO_ACK = 0x0800;

  // Clears both FIFO pointers. The remote writes this before every packet; clearing
  // only one side lets the pointers drift apart once TX and RX share the FIFO.
  constexpr uint16_t FIFO_CLEAR_TX_POINTER = 0x8000;
  constexpr uint16_t FIFO_CLEAR_RX_POINTER = 0x0080;
  constexpr uint16_t FIFO_CLEAR_BOTH = FIFO_CLEAR_TX_POINTER | FIFO_CLEAR_RX_POINTER;

  // Register 41 leaves PKT_FIFO_POLARITY clear: PKT goes high when TX completes
  // or a packet has been received.
  constexpr uint8_t PKT_ACTIVE_LEVEL = HIGH;
  // PKT is cleared as soon as TX is enabled; a packet takes far longer than this to send.
  constexpr uint32_t PKT_CLEAR_TIMEOUT_US = 1000;
  // With AUTO_ACK a packet takes about 11 ms from TX enable to PKT.
  constexpr uint32_t TX_TIMEOUT_MS = 100;

  // ---- Captured from the original remote's SPI bus (照明1 / 照明2) ----

  constexpr uint8_t CHANNEL = 74;  // 2402 + 74 = 2476 MHz
  static_assert(CHANNEL <= CHANNEL_MASK, "LT8920 channel must fit in 7 bits");

  // Registers 36..39. Register 32 selects a 32-bit sync word, so only 36 and 39
  // go on air; 37/38 are written as captured anyway.
  constexpr uint16_t SYNC_WORD[4] = {0x050A, 0x0380, 0x5A5A, 0x55AA};

  // CRC | PACK_LEN | FW_TERM_TX | AUTO_ACK (SDK: 0xB000, no AUTO_ACK).
  constexpr uint16_t REMOTE_PACKET_CONFIG = PACKET_CONFIG_CRC_ON |
    PACKET_CONFIG_PACK_LEN_ENABLE |
    PACKET_CONFIG_FW_TERM_TX |
    PACKET_CONFIG_AUTO_ACK;

  constexpr uint16_t REMOTE_TX_POWER_GAIN = 0x4800;
  constexpr uint16_t REMOTE_RSSI_CONTROL = 0x0008;
  constexpr uint16_t REMOTE_PREAMBLE_SYNC_CONFIG = 0x4800;
  constexpr uint16_t REMOTE_SYNC_THRESHOLD_CONFIG = 0x4402;
  constexpr uint16_t REMOTE_DATA_RATE_CONFIG = 0x0100;
  constexpr uint16_t REMOTE_DATA_RATE_OPTIONS = 0x0152;

  // Init writes that differ from the SDK's begin(), applied after it.
  constexpr Register REMOTE_REGISTERS[] = {
    {REG_CURRENT_CONTROL, REMOTE_TX_POWER_GAIN},  // SDK: 0x4000.
    {REG_RSSI_CONTROL, REMOTE_RSSI_CONTROL},      // SDK: 0x0000.
    {REG_PACKET_FORMAT, REMOTE_PREAMBLE_SYNC_CONFIG},  // 3-byte preamble, 32-bit sync word (SDK: 0x5000, 48-bit).
    {REG_SYNC_THRESHOLD, REMOTE_SYNC_THRESHOLD_CONFIG},  // SDK: 0x4401.
    {REG_PACKET_CONFIG, REMOTE_PACKET_CONFIG},
    {REG_DATA_RATE, REMOTE_DATA_RATE_CONFIG},  // 1 Mbps (SDK: 0x0101).
    {REG_DATA_RATE_OPTIONS, REMOTE_DATA_RATE_OPTIONS},  // Not written by the SDK.
  };

  // Re-arm RX when no packet has come in for this long. The chip has been seen
  // going deaf; a burst lasts ~170 ms with a packet every ~3 ms, so a quick
  // re-arm still catches most of a burst the chip stopped hearing, while a
  // re-arm that cuts into a packet costs one of ~50 copies.
  constexpr uint32_t RX_REARM_MS = 50;

  // RSSI sampling while idle, for the trace.
  constexpr uint32_t RSSI_SAMPLE_US = 1000;
  // Re-arms merged into one trace event (~1 s), so a long quiet keeps its timing.
  constexpr uint16_t MAX_REARM_RUN = 20;

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
  _listenConfigured = false;  // The SDK's begin() and REMOTE_REGISTERS write register 41.
  _model = _storage.load();
  _ownAddress = _model == nullptr ? 0 : _model->packet.remote;

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

uint8_t OppleFE6300::channel() const {
  return CHANNEL & CHANNEL_MASK;
}

size_t OppleFE6300::commandCount() const {
  return _model == nullptr ? 0 : _model->keys.size();
}

OppleFE6300::TxResult OppleFE6300::send(const String& name) {
  if (!_ready) {
    return TxResult::NotReady;
  }
  const auto key = _findKey(name);
  if (key == nullptr) {
    return TxResult::UnknownCommand;
  }
  auto fields = _model->packet;
  fields.key = key->code;
  fields.sequence = _nextSequence(_model->packet.sequence);
  uint8_t packet[OppleFE6300Packet::SIZE];
  fields.toBytes(packet);

  // The remote sends with AUTO_ACK; listening runs without it (see _listen()).
  _idle();
  _rf->writeRegister(REG_PACKET_CONFIG, REMOTE_PACKET_CONFIG);
  _listenConfigured = false;

  auto result = TxResult::Ok;
  for (uint8_t i = 0; i < _model->burstCount; i++) {
    const uint32_t startedMs = millis();
    result = _sendPacket(packet, sizeof(packet));
    if (result != TxResult::Ok) {
      break;
    }
    while (millis() - startedMs < _model->burstPeriodMs) {
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

  // Most packets are burst repeats (~50 per press). A repeat has the bytes of
  // the press already reported, so it has its fields too: only a new press is
  // split into fields.
  const uint32_t nowMs = millis();
  const bool repeat = packet.length == _lastRx.length &&
    memcmp(packet.payload, _lastRx.payload, packet.length) == 0 &&
    nowMs - _lastRxMs < REPEAT_WINDOW_MS;
  _lastRxMs = nowMs;
  if (repeat) {
    _record(TraceEvent::Kind::Packets, nowMs, _lastRx.sequence());
    _rxStats.lastCopies++;
    return false;
  }

  packet.remoteFormat = _model != nullptr &&
    OppleFE6300Packet::fromBytes(packet.payload, packet.length, packet.fields) &&
    packet.fields.sameFormat(_model->packet);
  _record(TraceEvent::Kind::Packets, nowMs, packet.sequence());
  packet.previousCopies = _rxStats.lastCopies;
  _rxStats.lastCopies = 1;

  packet.key = nullptr;
  packet.own = false;
  packet.skipped = 0;
  if (packet.remoteFormat) {
    packet.key = _findKey(packet.fields.key);
    // Unknown keys count too: they use up a counter value like any other press,
    // and skipping them would show the next known press as a skip.
    packet.skipped = _trackRemote(packet.address(), packet.sequence());
    packet.own = packet.address() == _ownAddress;
    if (packet.own) {
      _airSequence = packet.sequence();
      _hasAirSequence = true;
      _applyGapAfterRemote = true;
    }
  }
  _lastRx = packet;
  return true;
}

// Records the remote's counter and returns how many values it skipped. The
// counter moves forward by one per press; a step back or a jump of half the
// range or more is not counted.
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

uint32_t OppleFE6300::lastPacketMs() const {
  return _lastRxMs;
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

uint16_t OppleFE6300::sequence() const {
  return _hasAirSequence ? _airSequence : (_model != nullptr ? _model->packet.sequence : 0);
}

uint16_t OppleFE6300::RxPacket::sequence() const {
  return remoteFormat ? fields.sequence : 0;
}

uint32_t OppleFE6300::RxPacket::address() const {
  return remoteFormat ? fields.remote : 0;
}

const __FlashStringHelper* OppleFE6300::describe(TxResult result) {
  switch (result) {
    case TxResult::Ok:
      return F("TX complete");
    case TxResult::NotReady:
      return F("radio not ready");
    case TxResult::UnknownCommand:
      return F("unknown command; check opple-fe6300.json");
    case TxResult::PktMismatch:
      return F("PKT pin disagrees with status register; check PKT wiring/polarity");
    case TxResult::PktStuck:
      return F("PKT did not clear when TX started; check PKT wiring/polarity");
    case TxResult::Timeout:
      return F("TX timeout; check LT8920 connection and PKT pin");
  }
  return F("unknown");
}

const OppleFE6300Key* OppleFE6300::_findKey(const String& name) const {
  if (_model == nullptr) {
    return nullptr;
  }
  for (const auto& key : _model->keys) {
    if (key.name == name) {
      return &key;
    }
  }
  return nullptr;
}

const OppleFE6300Key* OppleFE6300::_findKey(uint8_t code) const {
  if (_model == nullptr) {
    return nullptr;
  }
  for (const auto& key : _model->keys) {
    if (key.code == code) {
      return &key;
    }
  }
  return nullptr;
}

// One new counter is used per command; repeated packets in that command's burst
// keep the same value. Start from the configured base, or apply the signed
// sequenceGap to the original remote's latest counter once, then advance by one
// per command. Counter arithmetic wraps modulo 16 bits.
uint16_t OppleFE6300::_nextSequence(uint16_t start) {
  const uint16_t base = _hasAirSequence ? _airSequence : start;
  const int32_t offset = _applyGapAfterRemote
    ? static_cast<int32_t>(_model->sequenceGap)
    : 1;
  const int32_t nextSequence = static_cast<int32_t>(base) + offset;
  const uint16_t sequence = static_cast<uint16_t>(nextSequence);
  _airSequence = sequence;
  _hasAirSequence = true;
  _applyGapAfterRemote = false;
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

// Passive listening: AUTO_ACK is cleared so this side never answers the
// remote's packets on air (answering also stopped the remote's retransmissions
// and made reception much worse). send() restores the remote's configuration.
// Register 41 is written only after something else changed it (begin(),
// send()), not on every re-arm: one SPI write less per packet and idle re-arm.
void OppleFE6300::_listen() {
  _idle();
  if (!_listenConfigured) {
    _rf->writeRegister(REG_PACKET_CONFIG, REMOTE_PACKET_CONFIG & ~PACKET_CONFIG_AUTO_ACK);
    _listenConfigured = true;
  }
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
