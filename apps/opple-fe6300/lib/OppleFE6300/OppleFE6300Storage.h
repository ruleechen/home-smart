#ifndef OppleFE6300Storage_h
#define OppleFE6300Storage_h

#include <vector>
#include <Arduino.h>
#include <Storage/FileStorage.h>
#include "OppleFE6300Packet.h"

// One key of the remote: the command name and its key code (packet byte 15).
struct OppleFE6300Key {
  String name;
  uint8_t code;
};

struct OppleFE6300Model {
  // Every field of the own remote's packet but the key: remote address, fixed
  // fields, and the sequence to start from (see below). key is left 0.
  OppleFE6300Packet packet;
  std::vector<OppleFE6300Key> keys;
  // Number of identical packets sent for each command; defaults for older configs.
  uint8_t burstCount = 15;
  // Minimum start-to-start interval between packets in a burst.
  uint16_t burstPeriodMs = 13;
  // Counter values to skip ahead after hearing the original remote.
  uint16_t sequenceGap = 100;
};

// Remote config in LittleFS (variant/<name>/data/opple-fe6300.json, packed into
// the FS image), containing packet fields and transmit settings:
//   { "packet": { "remote": "A3714D01", "prefix": "00000000",
//     "length": "0013", "sequence": "0000", "padding": "0000",
//     "type": "03", "keys": { "<name>": "37", ... },
//     "postfix": "005701" }, "burstCount": 15, "burstPeriodMs": 13,
//     "sequenceGap": 100 }
// Every value is hex with exactly the field's bytes (see OppleFE6300Packet).
// The sequence field is the remote's press counter: one value for the whole
// remote, +1 per press, so it is runtime state. The configured value is the
// initial base;
// each new command increments it. After a same-address remote packet, the next
// local command advances sequenceGap values past that counter before continuing
// by one.
// Packet and transmit settings are read directly from these configured values.
class OppleFE6300Storage : public Victor::Components::FileStorage<OppleFE6300Model> {
 public:
  explicit OppleFE6300Storage(const char* filePath = "/opple-fe6300.json");

 protected:
  void _serialize(const OppleFE6300Model* model, DynamicJsonDocument& doc) override;
  void _deserialize(OppleFE6300Model* model, const DynamicJsonDocument& doc) override;
};

#endif  // OppleFE6300Storage_h
