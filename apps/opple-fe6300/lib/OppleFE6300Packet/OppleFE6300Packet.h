#ifndef OppleFE6300Packet_h
#define OppleFE6300Packet_h

#include <Arduino.h>

// The packet the OPPLE FE6300 remote sends (see docs/saleae), as fields.
// This is the only place that knows where each field sits in the 19 bytes;
// storage, sending and receiving all go through toBytes()/fromBytes().
//
//   bytes   field    example        meaning
//   0..3    remote   A3 71 4D 01    remote address (another remote: A3 BF 57 01)
//   4..7    prefix   00 00 00 00    fixed
//   8..9    length   00 13          fixed (0x13 = 19, the payload length?)
//   10..11  counter  00 33          press counter, big-endian, one per remote,
//                                   +1 per press of any key, same for every
//                                   repeat of one press
//   12..13  padding  00 00          fixed
//   14      type     03             fixed
//   15      key      50             key code (37 light, 50 power off, 3F heat, ...)
//   16..18  postfix  00 57 01       fixed
//
// The fixed fields were the same on every key of both remotes seen.
struct OppleFE6300Packet {
  static constexpr size_t SIZE = 19;

  uint32_t remote = 0;
  uint8_t prefix[4] = {};
  uint8_t length[2] = {};
  uint16_t counter = 0;
  uint8_t padding[2] = {};
  uint8_t type = 0;
  uint8_t key = 0;
  uint8_t postfix[3] = {};

  void toBytes(uint8_t bytes[SIZE]) const;

  // False, leaving packet unspecified, unless length is exactly SIZE.
  static bool fromBytes(const uint8_t* bytes, size_t length, OppleFE6300Packet& packet);

  // True when the fixed fields (prefix, length, padding, type, postfix) are
  // equal: other is in the same format, whatever its remote, counter and key.
  bool sameFormat(const OppleFE6300Packet& other) const;
};

#endif  // OppleFE6300Packet_h
