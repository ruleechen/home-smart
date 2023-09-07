#ifndef GlobalHelpers_h
#define GlobalHelpers_h

#include <vector>
#include <Arduino.h>

namespace Victor {

  class GlobalHelpers {
   public:
    static String randomString(uint8_t length) {
      auto result = String(F(""));
      result.reserve(length);
      uint8_t generated = 0;
      while (generated < length) {
        // .platformio/packages/framework-arduinoespressif8266/cores/esp8266/WMath.cpp
        const byte randomValue = random(0, 26);
        char letter = randomValue + 'a';
        if (randomValue > 26) {
          letter = (randomValue - 26);
        }
        result += letter;
        generated++;
      }
      return result;
    }

    static std::vector<String> splitString(const String& str, const String& delimiter) {
      std::vector<String> parts;
      const auto delimiterLength = delimiter.length();
      auto searchFrom = 0;
      while (true) {
        const auto index = str.indexOf(delimiter, searchFrom);
        if (index == -1) {
          parts.push_back(str.substring(searchFrom));
          break;
        }
        parts.push_back(str.substring(searchFrom, index));
        searchFrom = index + delimiterLength;
      }
      return parts;
    }

    static String toYesNoName(bool state) {
      return state ? F("Yes") : F("No");
    }

    static String toOnOffName(bool state) {
      return state ? F("On") : F("Off");
    }

    static String toPercentage(float state) {
      return String(state) + F("%");
    }

    static String appendFilter(String value, String filter) {
      return value + F("|") + filter;
    }

    static String toMillisAgo(unsigned long state) {
      return appendFilter(String(state), F("ago"));
    }

  };

} // namespace Victor

#endif // GlobalHelpers_h
