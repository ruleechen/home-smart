#ifndef LT8920Radio_h
#define LT8920Radio_h

#include <Arduino.h>
#include <LT8920.h>

// Application-side wrapper around the LT8920 SDK.
// Owns SPI setup, chip configuration and a bounded, polarity-aware transmit,
// so callers only deal with channels, sync words and payloads.
// The SDK under lib/LT8920 is used as-is and stays unmodified.
class LT8920Radio {
 public:
  struct Pins {
    uint8_t cs;   // Chip select.
    uint8_t pkt;  // PKT flag input.
    uint8_t rst;  // Reset output; the SDK always drives this GPIO.
  };

  enum class SyncWordLength : uint8_t {
    Bits16 = 0,
    Bits32 = 1,
    Bits48 = 2,
    Bits64 = 3,
  };

  struct Config {
    uint8_t channel = 0x31;  // Frequency = 2402 + channel MHz, 7 bits.
    LT8920::DataRate dataRate = LT8920::LT8920_1MBPS;
    SyncWordLength syncWordLength = SyncWordLength::Bits64;
    uint16_t syncWord[4] = {0, 0, 0, 0};  // Values for registers 36..39, in register order.
    uint8_t pktActiveLevel = HIGH;        // SDK default: PKT goes high when TX completes.
    uint32_t txTimeoutMs = 100;
  };

  enum class TxResult : uint8_t {
    Ok,
    NotReady,       // begin() was not called or its register check failed.
    InvalidLength,  // Payload is empty or does not fit the 64-byte FIFO.
    PktStuck,       // PKT stayed active after the FIFO reset: wiring/polarity.
    Timeout,        // PKT never became active within txTimeoutMs.
  };

  // Largest payload that fits the FIFO together with its length byte.
  static constexpr size_t MAX_PAYLOAD = 63;

  explicit LT8920Radio(const Pins& pins);
  ~LT8920Radio();

  // Initializes SPI, resets and configures the chip, then reads registers back.
  // Returns false when the chip does not answer as configured; send() is then refused.
  bool begin(const Config& config);
  bool isReady() const;
  uint8_t channel() const;

  // Sends one packet. The length byte is prepended here (register 41 has
  // PACK_LEN_ENABLE set), so pass the payload only.
  TxResult send(const uint8_t* payload, size_t length);
  template <size_t N>
  TxResult send(const uint8_t (&payload)[N]) {
    return send(payload, N);
  }

  static const __FlashStringHelper* describe(TxResult result);

  // Escape hatch for register-level experiments; nullptr before begin().
  LT8920* sdk();

 private:
  Pins _pins;
  Config _config;
  LT8920* _rf = nullptr;
  bool _ready = false;

  void _idle();
  void _flushTx();
  bool _isPktActive() const;
  void _writeSyncWord();
  bool _verifySyncWord();
};

#endif  // LT8920Radio_h
