#ifndef OppleHelper_h
#define OppleHelper_h

#include <Arduino.h>

// Small helpers shared by the app and the Opple libs.
namespace OppleHelper {

  // Upper-case hex digits, two per byte, no separators: "A3714D01".
  String hex(const uint8_t* bytes, size_t length);
  String hex16(uint16_t value);  // Big-endian, 4 digits.
  String hex32(uint32_t value);  // Big-endian, 8 digits.

  // Parses exactly size bytes of hex into out. False for null, any other
  // length or a non-hex digit (out is then unspecified); no allocation.
  bool parseHex(const char* text, uint8_t* out, size_t size);

  // Big-endian integers inside a byte buffer.
  uint16_t readBE16(const uint8_t* bytes);
  uint32_t readBE32(const uint8_t* bytes);
  void writeBE16(uint8_t* bytes, uint16_t value);
  void writeBE32(uint8_t* bytes, uint32_t value);

} // namespace OppleHelper

#endif  // OppleHelper_h
