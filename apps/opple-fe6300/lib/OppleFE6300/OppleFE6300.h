#ifndef OppleFE6300_h
#define OppleFE6300_h

#include <Arduino.h>
#include <LT8920.h>
#include <OppleFE6300Storage.h>

// OPPLE FE6300 remote emulation on top of the LT8920 SDK.
// Holds the RF setup captured from the original remote (registers, sync word,
// channel, repeat burst); the remote's packet fields and key codes come from
// OppleFE6300Storage, so callers only pick a command by name.
// Between transmissions the chip listens on the remote's channel, so presses on
// the original remote can be picked up with receive().
// The SDK under lib/LT8920 is the generic driver and stays unmodified.
//
// Packet fields: see OppleFE6300Packet. Every key is a toggle: on and off are
// the same packet with a new counter. Key code 4D replaces about every 4th of
// rapid light presses, and the heater ignores it.
// A received packet in the configured format is recognized by its key code
// whatever its remote and counter, so keys of any remote are recognized; own
// tells the configured remote apart.
class OppleFE6300 {
 public:
  struct Pins {
    uint8_t cs;   // Chip select.
    uint8_t pkt;  // PKT flag input.
    uint8_t rst;  // Reset output; the SDK always drives this GPIO.
  };

  enum class TxResult : uint8_t {
    Ok,
    NotReady,        // begin() was not called or its register check failed.
    UnknownCommand,  // No key with that name in opple-fe6300.json.
    PktMismatch,     // PKT GPIO disagrees with the PKT_FLAG status bit: wiring/polarity.
    PktStuck,     // PKT did not clear when TX was enabled.
    Timeout,      // PKT never became active within the TX timeout.
  };

  // Largest payload that fits the FIFO together with its length byte.
  static constexpr size_t MAX_PAYLOAD = 63;

  // One key press heard on air (the remote repeats each press ~12 times;
  // receive() reports it once).
  struct RxPacket {
    uint8_t length;                // Raw payload as received.
    uint8_t payload[MAX_PAYLOAD];
    // In the configured format (OppleFE6300Packet::sameFormat); only then are
    // fields, key, own and skipped set.
    bool remoteFormat;
    OppleFE6300Packet fields;
    const OppleFE6300Key* key;  // Configured key with this code, nullptr if unknown.
    bool own;  // Sent by the configured remote.
    // Counter values this remote skipped since its previous press heard.
    // Mostly the remote's own doing: on rapid presses it uses up values it
    // never sends, so the heater does not get them either. Otherwise they are
    // presses this side did not hear (while sending, or deaf), which may leave
    // the inferred state stale; the trace tells the two apart.
    uint16_t skipped;
    uint32_t previousCopies;  // Copies received of the previous press.
    uint16_t sequence() const;  // Press counter; 0 unless remoteFormat.
    uint32_t address() const;   // Remote address; 0 unless remoteFormat.
  };

  explicit OppleFE6300(const Pins& pins, const char* commandsPath = "/opple-fe6300.json");
  ~OppleFE6300();

  // Loads the remote config (LittleFS must already be mounted), initializes SPI,
  // resets the chip and applies the remote's configuration, then reads registers
  // back. Returns false when the chip does not answer as configured; send() is
  // then refused. A missing or invalid config does not fail begin():
  // commandCount() is 0 and send() reports UnknownCommand.
  bool begin();
  uint8_t channel() const;
  size_t commandCount() const;

  // Sends the named command the way the remote does: the same packet repeated
  // in a burst. Returns the first failure, or Ok when every packet went out.
  // The press counter is chosen so the heater does not drop it as a repeat of
  // the last press, and so the remote's next press is not dropped either.
  // The radio goes back to listening afterwards, whatever the result.
  TxResult send(const String& name);

  // Polls for a packet received while listening. Returns true once per key
  // press, with repeats of the same press dropped. Call it often from loop():
  // the chip holds one packet and is re-armed only here.
  bool receive(RxPacket& packet);

  // Receive counters since boot.
  struct RxStats {
    uint32_t packets;     // Valid packets, burst repeats included.
    uint32_t crcErrors;
    uint32_t empty;       // Zero-length packets (not errors; likely ACK frames).
    uint32_t oversize;    // Length byte beyond MAX_PAYLOAD.
    uint32_t lastCopies;  // Copies received of the latest press (the remote sends ~12).
  };
  const RxStats& rxStats() const;
  // millis() of the latest valid packet, burst repeats included; 0 before any.
  uint32_t lastPacketMs() const;

  // What the receiver went through lately, for telling why a press was missed.
  // Consecutive events of one kind are merged into a run (packets of one press,
  // CRC errors, idle re-arms up to ~1 s each), oldest first.
  struct TraceEvent {
    enum class Kind : uint8_t {
      Packets,   // Valid packets of one press counter.
      CrcError,
      Empty,
      Oversize,
      Rearm,     // Idle re-arm: no packet for RX_REARM_MS.
    };
    Kind kind;
    uint16_t count;
    uint16_t sequence;  // Packets: the press counter.
    uint16_t stuck;     // Rearm: re-arms that found a sync word but no packet.
    // Highest RSSI sampled while idle during the run. Between the packets of a
    // press it is the level of the remote on air; a re-arm run as loud as that
    // means something was sending that this side did not receive.
    uint8_t rssiMax;
    uint32_t firstMs;
    uint32_t lastMs;
  };
  static constexpr size_t TRACE_SIZE = 64;
  size_t traceCount() const;
  const TraceEvent& traceAt(size_t index) const;  // 0 is the oldest.

  // Register snapshot for bring-up on the serial console.
  struct Diagnostics {
    bool ready;
    bool pktPin;               // PKT GPIO at its active level.
    uint16_t channelRegister;  // Reg 7: bit 8 TX, bit 7 RX, bits 6:0 channel.
    uint16_t packetConfig;     // Reg 41.
    uint16_t status;           // Reg 48: bit 15 CRC error, bit 6 PKT flag.
    uint16_t fifoControl;      // Reg 52: write pointer 13:8, read pointer 5:0.
    uint8_t rssi;
    RxStats rx;
    bool hasTxSequence;
    uint16_t txSequence;       // Counter of the last own-remote packet on air, from either side.
  };
  Diagnostics diagnostics();

  static const __FlashStringHelper* describe(TxResult result);

 private:
  Pins _pins;
  OppleFE6300Storage _storage;
  const OppleFE6300Model* _model = nullptr;
  LT8920* _rf = nullptr;
  bool _ready = false;

  // Last packet reported by receive(), for dropping the rest of its burst.
  RxPacket _lastRx = {};
  uint32_t _lastRxMs = 0;
  RxStats _rxStats = {};

  // Re-arm time, for the idle re-arm in receive().
  uint32_t _armedMs = 0;
  // Register 41 holds the listening config (no AUTO_ACK). Cleared whenever
  // something else writes it, so _listen() writes it only then.
  bool _listenConfigured = false;

  TraceEvent _trace[TRACE_SIZE] = {};
  size_t _traceStart = 0;
  size_t _traceCount = 0;
  uint8_t _idleRssiMax = 0;  // Since the last trace event.
  uint32_t _rssiSampledUs = 0;

  // Latest counter heard from each remote, for spotting skipped values.
  struct RemoteCounter {
    uint32_t address;
    uint16_t sequence;
  };
  static constexpr size_t MAX_REMOTES = 4;
  RemoteCounter _remotes[MAX_REMOTES] = {};
  size_t _remoteCount = 0;
  size_t _nextRemoteSlot = 0;

  // Own remote (the configured address): its latest counter, and the
  // latest counter on air with its address, from it or from this side.
  uint32_t _ownAddress = 0;
  bool _hasRemoteSequence = false;
  uint16_t _remoteSequence = 0;
  bool _hasAirSequence = false;
  uint16_t _airSequence = 0;

  const OppleFE6300Key* _findKey(const String& name) const;
  const OppleFE6300Key* _findKey(uint8_t code) const;
  uint16_t _trackRemote(uint32_t address, uint16_t sequence);
  uint16_t _nextSequence(uint16_t start);
  TxResult _sendPacket(const uint8_t* payload, size_t length);
  void _record(TraceEvent::Kind kind, uint32_t ms, uint16_t sequence = 0, bool stuck = false);
  void _listen();
  void _idle();
  void _flushTx();
  bool _isPktActive() const;
  bool _pktFlagRegister();
  bool _verifySyncWord();
};

#endif  // OppleFE6300_h
