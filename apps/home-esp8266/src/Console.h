#ifndef Console_h
#define Console_h

#include <Arduino.h>

namespace Victor {
  class Console {
   public:
    Console();
    void begin(unsigned long baud);
    Console log();
    Console log(const String& msg);
    Console error();
    Console error(const String& msg);
    Console debug();
    Console debug(const String& msg);
    Console ts();
    Console bracket(const String& msg);
    Console bracket(unsigned long msg);
    Console section(const String& key, const String& val);
    Console section(const String& msg);
    Console write(const String& msg);
    Console write(unsigned long msg);
    Console newline();
  };

  // global
  extern Console console;

} // namespace Victor

#endif // Console_h
