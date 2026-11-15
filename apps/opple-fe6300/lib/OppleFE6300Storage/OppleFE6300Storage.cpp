#include "OppleFE6300Storage.h"
#include <OppleHelper.h>

using OppleHelper::hex;
using OppleHelper::parseHex;

OppleFE6300Storage::OppleFE6300Storage(const char* filePath) : FileStorage(filePath) {
  // About 20 bytes per key plus ~150 for the fixed fields: room for far more keys than the remote has.
  _maxSize = 4096;
}

void OppleFE6300Storage::_serialize(const OppleFE6300Model* model, DynamicJsonDocument& doc) {
  const auto& p = model->packet;
  doc[F("remote")] = OppleHelper::hex32(p.remote);
  doc[F("prefix")] = hex(p.prefix, sizeof(p.prefix));
  doc[F("length")] = hex(p.length, sizeof(p.length));
  doc[F("counter")] = OppleHelper::hex16(p.counter);
  doc[F("padding")] = hex(p.padding, sizeof(p.padding));
  doc[F("type")] = hex(&p.type, sizeof(p.type));
  const JsonObject keys = doc.createNestedObject(F("key"));
  for (const auto& key : model->keys) {
    keys[key.name] = hex(&key.code, sizeof(key.code));
  }
  doc[F("postfix")] = hex(p.postfix, sizeof(p.postfix));
}

void OppleFE6300Storage::_deserialize(OppleFE6300Model* model, const DynamicJsonDocument& doc) {
  // Reads one field of exactly size bytes into out; false (and reported) when
  // missing or malformed.
  const auto readField = [&](const __FlashStringHelper* name, uint8_t* out, size_t size) {
    const char* text = doc[name];
    std::vector<uint8_t> bytes;
    if (parseHex(text, bytes) && bytes.size() == size) {
      memcpy(out, bytes.data(), size);
      return true;
    }
    _error().section(F("invalid field"), String(name) + '=' + (text == nullptr ? "(missing)" : text));
    return false;
  };

  auto& p = model->packet;
  uint8_t remote[4];
  uint8_t counter[2];
  // Every field is read, so all bad ones are reported at once.
  bool valid = readField(F("remote"), remote, sizeof(remote));
  valid = readField(F("prefix"), p.prefix, sizeof(p.prefix)) && valid;
  valid = readField(F("length"), p.length, sizeof(p.length)) && valid;
  valid = readField(F("counter"), counter, sizeof(counter)) && valid;
  valid = readField(F("padding"), p.padding, sizeof(p.padding)) && valid;
  valid = readField(F("type"), &p.type, sizeof(p.type)) && valid;
  valid = readField(F("postfix"), p.postfix, sizeof(p.postfix)) && valid;
  if (!valid) {
    return;
  }
  p.remote = OppleHelper::readBE32(remote);
  p.counter = OppleHelper::readBE16(counter);

  const JsonObjectConst keys = doc[F("key")];
  for (const JsonPairConst item : keys) {
    std::vector<uint8_t> code;
    if (!parseHex(item.value().as<const char*>(), code) || code.size() != 1) {
      _error().section(F("invalid key"), item.key().c_str());
      continue;
    }
    model->keys.push_back({item.key().c_str(), code[0]});
  }
}
