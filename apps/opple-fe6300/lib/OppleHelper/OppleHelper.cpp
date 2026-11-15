#include "OppleHelper.h"

namespace OppleHelper {

  namespace {
    int hexDigit(char c) {
      if (c >= '0' && c <= '9') return c - '0';
      if (c >= 'A' && c <= 'F') return c - 'A' + 10;
      if (c >= 'a' && c <= 'f') return c - 'a' + 10;
      return -1;
    }
  } // namespace

  String hex(const uint8_t* bytes, size_t length) {
    static const char DIGITS[] = "0123456789ABCDEF";
    String text;
    text.reserve(length * 2);
    for (size_t i = 0; i < length; i++) {
      text += DIGITS[bytes[i] >> 4];
      text += DIGITS[bytes[i] & 0x0F];
    }
    return text;
  }

  String hex16(uint16_t value) {
    uint8_t bytes[2];
    writeBE16(bytes, value);
    return hex(bytes, 2);
  }

  String hex32(uint32_t value) {
    uint8_t bytes[4];
    writeBE32(bytes, value);
    return hex(bytes, 4);
  }

  bool parseHex(const char* text, std::vector<uint8_t>& out) {
    if (text == nullptr) {
      return false;
    }
    const size_t length = strlen(text);
    if (length == 0 || length % 2 != 0) {
      return false;
    }
    out.reserve(out.size() + length / 2);
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

  uint16_t readBE16(const uint8_t* bytes) {
    return (static_cast<uint16_t>(bytes[0]) << 8) | bytes[1];
  }

  uint32_t readBE32(const uint8_t* bytes) {
    return (static_cast<uint32_t>(bytes[0]) << 24) |
      (static_cast<uint32_t>(bytes[1]) << 16) |
      (static_cast<uint32_t>(bytes[2]) << 8) |
      bytes[3];
  }

  void writeBE16(uint8_t* bytes, uint16_t value) {
    bytes[0] = value >> 8;
    bytes[1] = value & 0xFF;
  }

  void writeBE32(uint8_t* bytes, uint32_t value) {
    writeBE16(bytes, value >> 16);
    writeBE16(bytes + 2, value & 0xFFFF);
  }

} // namespace OppleHelper
