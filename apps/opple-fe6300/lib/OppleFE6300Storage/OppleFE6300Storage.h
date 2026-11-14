#ifndef OppleFE6300Storage_h
#define OppleFE6300Storage_h

#include <vector>
#include <Arduino.h>
#include <Storage/FileStorage.h>

// One remote command: FIFO payload without the leading length byte,
// which OppleFE6300 prepends because register 41 has PACK_LEN_ENABLE set.
struct OppleFE6300Command {
  String name;
  std::vector<uint8_t> payload;
};

struct OppleFE6300Model {
  std::vector<OppleFE6300Command> commands;
};

// Command table in LittleFS (variant/<name>/data/opple-fe6300.json, packed into the FS image):
//   { "commands": { "<name>": "A3714D01...", ... } }
// Each payload is one hex string, as read from the logic analyzer.
// Commands with malformed hex are skipped and reported on the console.
class OppleFE6300Storage : public Victor::Components::FileStorage<OppleFE6300Model> {
 public:
  explicit OppleFE6300Storage(const char* filePath = "/opple-fe6300.json");

 protected:
  void _serialize(const OppleFE6300Model* model, DynamicJsonDocument& doc) override;
  void _deserialize(OppleFE6300Model* model, const DynamicJsonDocument& doc) override;
};

#endif  // OppleFE6300Storage_h
