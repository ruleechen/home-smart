#include "OppleFE6300Packet.h"
#include <OppleHelper.h>

namespace {

  // Field offsets, in packet order.
  constexpr size_t REMOTE_AT = 0;
  constexpr size_t PREFIX_AT = 4;
  constexpr size_t LENGTH_AT = 8;
  constexpr size_t COUNTER_AT = 10;
  constexpr size_t PADDING_AT = 12;
  constexpr size_t TYPE_AT = 14;
  constexpr size_t KEY_AT = 15;
  constexpr size_t POSTFIX_AT = 16;

  template <size_t N>
  bool equal(const uint8_t (&a)[N], const uint8_t (&b)[N]) {
    return memcmp(a, b, N) == 0;
  }

} // namespace

void OppleFE6300Packet::toBytes(uint8_t bytes[SIZE]) const {
  OppleHelper::writeBE32(bytes + REMOTE_AT, remote);
  memcpy(bytes + PREFIX_AT, prefix, sizeof(prefix));
  memcpy(bytes + LENGTH_AT, length, sizeof(length));
  OppleHelper::writeBE16(bytes + COUNTER_AT, counter);
  memcpy(bytes + PADDING_AT, padding, sizeof(padding));
  bytes[TYPE_AT] = type;
  bytes[KEY_AT] = key;
  memcpy(bytes + POSTFIX_AT, postfix, sizeof(postfix));
}

bool OppleFE6300Packet::fromBytes(const uint8_t* bytes, size_t length, OppleFE6300Packet& packet) {
  if (length != SIZE) {
    return false;
  }
  packet.remote = OppleHelper::readBE32(bytes + REMOTE_AT);
  memcpy(packet.prefix, bytes + PREFIX_AT, sizeof(packet.prefix));
  memcpy(packet.length, bytes + LENGTH_AT, sizeof(packet.length));
  packet.counter = OppleHelper::readBE16(bytes + COUNTER_AT);
  memcpy(packet.padding, bytes + PADDING_AT, sizeof(packet.padding));
  packet.type = bytes[TYPE_AT];
  packet.key = bytes[KEY_AT];
  memcpy(packet.postfix, bytes + POSTFIX_AT, sizeof(packet.postfix));
  return true;
}

bool OppleFE6300Packet::sameFormat(const OppleFE6300Packet& other) const {
  return equal(prefix, other.prefix) &&
    equal(length, other.length) &&
    equal(padding, other.padding) &&
    type == other.type &&
    equal(postfix, other.postfix);
}
