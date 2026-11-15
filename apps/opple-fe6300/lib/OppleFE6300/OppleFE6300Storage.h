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
  // fields, and the counter to start from (see below). key is left 0.
  OppleFE6300Packet packet;
  std::vector<OppleFE6300Key> keys;
};

// Remote config in LittleFS (variant/<name>/data/opple-fe6300.json, packed into
// the FS image), one entry per packet field, in packet order:
//   { "remote": "A3714D01", "prefix": "00000000", "length": "0013",
//     "counter": "0000", "padding": "0000", "type": "03",
//     "key": { "<name>": "37", ... }, "postfix": "005701" }
// Every value is hex with exactly the field's bytes (see OppleFE6300Packet).
// The counter is listed for readability: one counter for the whole remote, +1
// per press, so it is runtime state. OppleFE6300 sets it when sending and uses
// the configured value only as a start until the remote has been heard.
// A malformed or missing field loads no key at all; a malformed key code skips
// that key. Both are reported on the console.
class OppleFE6300Storage : public Victor::Components::FileStorage<OppleFE6300Model> {
 public:
  explicit OppleFE6300Storage(const char* filePath = "/opple-fe6300.json");

 protected:
  void _serialize(const OppleFE6300Model* model, DynamicJsonDocument& doc) override;
  void _deserialize(OppleFE6300Model* model, const DynamicJsonDocument& doc) override;
};

#endif  // OppleFE6300Storage_h
