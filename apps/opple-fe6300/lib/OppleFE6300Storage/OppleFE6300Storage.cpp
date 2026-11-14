#include "OppleFE6300Storage.h"

namespace {

  int hexDigit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
  }

  // Accepts an even number of hex digits, e.g. "A3714D01".
  bool parseHex(const char* text, std::vector<uint8_t>& out) {
    if (text == nullptr) {
      return false;
    }
    const size_t length = strlen(text);
    if (length == 0 || length % 2 != 0) {
      return false;
    }
    out.reserve(length / 2);
    for (size_t i = 0; i < length; i += 2) {
      const int high = hexDigit(text[i]);
      const int low = hexDigit(text[i + 1]);
      if (high < 0 || low < 0) {
        return false;
      }
      out.push_back(static_cast<uint8_t>((high << 4) | low));
    }
    return true;
  }

  String toHex(const std::vector<uint8_t>& bytes) {
    static const char DIGITS[] = "0123456789ABCDEF";
    String text;
    text.reserve(bytes.size() * 2);
    for (const auto value : bytes) {
      text += DIGITS[value >> 4];
      text += DIGITS[value & 0x0F];
    }
    return text;
  }

} // namespace

OppleFE6300Storage::OppleFE6300Storage(const char* filePath) : FileStorage(filePath) {
  // About 70 bytes per command (key + 19-byte hex string): room for ~50 commands.
  _maxSize = 4096;
}

void OppleFE6300Storage::_serialize(const OppleFE6300Model* model, DynamicJsonDocument& doc) {
  const JsonObject commands = doc.createNestedObject(F("commands"));
  for (const auto& command : model->commands) {
    commands[command.name] = toHex(command.payload);
  }
}

void OppleFE6300Storage::_deserialize(OppleFE6300Model* model, const DynamicJsonDocument& doc) {
  const JsonObjectConst commands = doc[F("commands")];
  for (const JsonPairConst item : commands) {
    OppleFE6300Command command;
    command.name = item.key().c_str();
    if (!parseHex(item.value().as<const char*>(), command.payload)) {
      _error().section(F("invalid hex"), command.name);
      continue;
    }
    model->commands.push_back(std::move(command));
  }
}
