#ifndef OppleFE6300_h
#define OppleFE6300_h

#include <Arduino.h>
#include <LT8920.h>
#include <OppleFE6300Storage.h>

// OPPLE FE6300 remote emulation on top of the LT8920 SDK.
// Holds the RF setup captured from the original remote (registers, sync word,
// channel, repeat burst); command payloads come from OppleFE6300Storage, so
// callers only pick a command by name.
// Between transmissions the chip listens on the remote's channel, so presses on
// the original remote can be picked up with receive().
// The SDK under lib/LT8920 is the generic driver and stays unmodified.
//
// Payload layout (19 bytes, see docs/saleae):
//   [0..3]   A3 71 4D 01  remote address (another remote seen: A3 BF 57 01)
//   [10..11] press counter, 16-bit big-endian, +1 per key press, same for every
//            repeat of one press (the other remote was at 0x113E)
//   [15]     key code (37 light toggle, 50 power off, 3F heat, ...); 4D replaces
//            about every 4th of rapid light presses and the heater ignores it
// The light key is a toggle: on and off are the same packet with a new counter.
// Stored commands match a received packet whatever its address and counter, so
// keys of any remote are recognized; own() tells the stored remote apart.
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
    UnknownCommand,  // No command with that name in opple-fe6300.json.
    InvalidPayload,  // Payload is empty or does not fit the 64-byte FIFO.
    PktMismatch,     // PKT GPIO disagrees with the PKT_FLAG status bit: wiring/polarity.
    PktStuck,     // PKT did not clear when TX was enabled.
    Timeout,      // PKT never became active within the TX timeout.
  };

  // Largest payload that fits the FIFO together with its length byte.
  static constexpr size_t MAX_PAYLOAD = 63;

  // One key press heard on air (the remote repeats each press ~12 times;
  // receive() reports it once).
  struct RxPacket {
    const OppleFE6300Command* command;  // Matching stored command, nullptr if none.
    uint8_t length;
    uint8_t payload[MAX_PAYLOAD];
    bool own;  // Sent by the remote whose address the stored commands carry.
    // Presses this remote's counter skipped since its previous one heard:
    // presses made while this side was not listening (sending, watch, scan) or
    // could not hear. Any key may be among them, so inferred state may be stale.
    uint16_t missed;
    uint32_t previousCopies;  // Copies received of the previous press.
    uint16_t sequence() const;  // Press counter; 0 when the packet is too short to carry one.
    uint32_t address() const;   // Remote address; 0 when the packet is too short.
  };

  explicit OppleFE6300(const Pins& pins, const char* commandsPath = "/opple-fe6300.json");
  ~OppleFE6300();

  // Loads the commands (LittleFS must already be mounted), initializes SPI,
  // resets the chip and applies the remote's configuration, then reads registers
  // back. Returns false when the chip does not answer as configured; send() is
  // then refused. A missing command file does not fail begin(): commandCount()
  // is 0 and send() reports UnknownCommand.
  bool begin();
  bool isReady() const;
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

  // Polls the status register for durationMs while listening (press the remote
  // meanwhile) and counts what the receiver gets as far as: RF energy (RSSI),
  // sync word match, complete packet, CRC result. Packets it sees are discarded.
  struct WatchResult {
    uint32_t samples;
    uint8_t rssiMin;
    uint8_t rssiMax;
    uint32_t syncWords;  // Rising edges of the SYNCWORD_RECV status bit.
    uint32_t packets;    // PKT_FLAG set, CRC good or bad.
    uint32_t crcErrors;
    uint32_t pktPin;     // Packets whose PKT GPIO was also active.
  };
  WatchResult watch(uint32_t durationMs);

  // Sweeps RX over channels 0..CHANNEL_COUNT-1 for durationMs, keeping the
  // highest RSSI per channel. Press the remote meanwhile to see where it sends.
  static constexpr uint8_t CHANNEL_COUNT = 84;  // 2402..2485 MHz
  void scan(uint32_t durationMs, uint8_t maxRssi[CHANNEL_COUNT]);

  static const __FlashStringHelper* describe(TxResult result);

  // Experiment: listen with AUTO_ACK on (as the remote sends) instead of off.
  // On, this side answers every packet it receives with an ACK frame, which
  // also stops the remote's retransmissions. Off by default.
  void setRxAutoAck(bool enabled);
  bool rxAutoAck() const;

  // Escape hatch for register-level experiments; nullptr before begin().
  LT8920* sdk();

 private:
  Pins _pins;
  OppleFE6300Storage _storage;
  const OppleFE6300Model* _commands = nullptr;
  LT8920* _rf = nullptr;
  bool _ready = false;

  // Last packet reported by receive(), for dropping the rest of its burst.
  RxPacket _lastRx = {};
  uint32_t _lastRxMs = 0;
  RxStats _rxStats = {};

  bool _rxAutoAck = false;

  // Re-arm time, for the idle re-arm in receive().
  uint32_t _armedMs = 0;

  TraceEvent _trace[TRACE_SIZE] = {};
  size_t _traceStart = 0;
  size_t _traceCount = 0;
  uint8_t _idleRssiMax = 0;  // Since the last trace event.
  uint32_t _rssiSampledUs = 0;

  // Latest counter heard from each remote, for spotting missed presses.
  struct RemoteCounter {
    uint32_t address;
    uint16_t sequence;
  };
  static constexpr size_t MAX_REMOTES = 4;
  RemoteCounter _remotes[MAX_REMOTES] = {};
  size_t _remoteCount = 0;
  size_t _nextRemoteSlot = 0;

  // Own remote (the stored commands' address): its latest counter, and the
  // latest counter on air with its address, from it or from this side.
  uint32_t _ownAddress = 0;
  bool _hasRemoteSequence = false;
  uint16_t _remoteSequence = 0;
  bool _hasAirSequence = false;
  uint16_t _airSequence = 0;

  const OppleFE6300Command* _findCommand(const String& name) const;
  const OppleFE6300Command* _matchCommand(const uint8_t* payload, size_t length) const;
  uint16_t _trackRemote(uint32_t address, uint16_t sequence);
  uint16_t _nextSequence(uint16_t templateSequence);
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
