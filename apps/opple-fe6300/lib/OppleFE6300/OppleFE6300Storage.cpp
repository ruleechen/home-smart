#include "OppleFE6300Storage.h"
#include <OppleHelper.h>

using OppleHelper::hex;
using OppleHelper::parseHex;

OppleFE6300Storage::OppleFE6300Storage(const char* filePath) : FileStorage(filePath) {
  // About 20 bytes per key plus ~150 for the fixed fields: room for far more keys than the remote has.
  _maxSize = 4096;
}

void OppleFE6300Storage::_serialize(const OppleFE6300Model* model, DynamicJsonDocument& doc) {
  JsonObject packet = doc.createNestedObject(F("packet"));
  const auto& p = model->packet;
  packet[F("remote")] = OppleHelper::hex32(p.remote);
  packet[F("prefix")] = hex(p.prefix, sizeof(p.prefix));
  packet[F("length")] = hex(p.length, sizeof(p.length));
  packet[F("counter")] = OppleHelper::hex16(p.counter);
  packet[F("padding")] = hex(p.padding, sizeof(p.padding));
  packet[F("type")] = hex(&p.type, sizeof(p.type));

  const JsonObject keys = packet.createNestedObject(F("keys"));
  for (const auto& key : model->keys) {
    keys[key.name] = hex(&key.code, sizeof(key.code));
  }
  packet[F("postfix")] = hex(p.postfix, sizeof(p.postfix));

  // others
  doc[F("burstCount")] = model->burstCount;
  doc[F("burstPeriodMs")] = model->burstPeriodMs;
}

void OppleFE6300Storage::_deserialize(OppleFE6300Model* model, const DynamicJsonDocument& doc) {
  const JsonObjectConst packet = doc[F("packet")];
  auto& p = model->packet;
  uint8_t remote[4];
  uint8_t counter[2];
  parseHex(packet[F("remote")], remote, sizeof(remote));
  parseHex(packet[F("prefix")], p.prefix, sizeof(p.prefix));
  parseHex(packet[F("length")], p.length, sizeof(p.length));
  parseHex(packet[F("counter")], counter, sizeof(counter));
  parseHex(packet[F("padding")], p.padding, sizeof(p.padding));
  parseHex(packet[F("type")], &p.type, sizeof(p.type));
  parseHex(packet[F("postfix")], p.postfix, sizeof(p.postfix));
  p.remote = OppleHelper::readBE32(remote);
  p.counter = OppleHelper::readBE16(counter);

  const JsonObjectConst keys = packet[F("keys")];
  for (const JsonPairConst item : keys) {
    uint8_t code;
    parseHex(item.value().as<const char*>(), &code, sizeof(code));
    model->keys.push_back({item.key().c_str(), code});
  }

  // others
  model->burstCount = doc[F("burstCount")].as<uint8_t>();
  model->burstPeriodMs = doc[F("burstPeriodMs")].as<uint16_t>();
}
