#ifndef OppleFE6300_h
#define OppleFE6300_h

#include <Arduino.h>
#include <LT8920.h>
#include <OppleFE6300Storage.h>

// OPPLE FE6300 remote emulation on top of the LT8920 SDK.
// Holds the RF setup captured from the original remote (registers, sync word,
// channel, repeat burst); command payloads come from OppleFE6300Storage, so
// callers only pick a command by name.
// The SDK under lib/LT8920 is the generic driver and stays unmodified.
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
  TxResult send(const String& name);

  static const __FlashStringHelper* describe(TxResult result);

  // Escape hatch for register-level experiments; nullptr before begin().
  LT8920* sdk();

 private:
  Pins _pins;
  OppleFE6300Storage _storage;
  const OppleFE6300Model* _commands = nullptr;
  LT8920* _rf = nullptr;
  bool _ready = false;

  const OppleFE6300Command* _findCommand(const String& name) const;
  TxResult _sendPacket(const uint8_t* payload, size_t length);
  void _idle();
  void _flushTx();
  bool _isPktActive() const;
  bool _pktFlagRegister();
  bool _verifySyncWord();
};

#endif  // OppleFE6300_h
