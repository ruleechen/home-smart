#include "Console.h"

namespace Victor {

  Console::Console() {}

  void Console::begin(unsigned long baud) {
    Serial.begin(baud);
  }

  Console Console::log() {
    return ts().bracket(F("log"));
  }

  Console Console::log(const String& msg) {
    return log().section(msg);
  }

  Console Console::error() {
    return ts().bracket(F("error"));
  }

  Console Console::error(const String& msg) {
    return error().section(msg);
  }

  Console Console::debug() {
    return ts().bracket(F("debug"));
  }

  Console Console::debug(const String& msg) {
    return debug().section(msg);
  }

  Console Console::ts() {
    return newline().bracket(millis());
  }

  Console Console::bracket(const String& msg) {
    return write(F("[")).write(msg).write(F("]"));
  }

  Console Console::bracket(unsigned long msg) {
    return write(F("[")).write(msg).write(F("]"));
  }

  Console Console::section(const String& key, const String& val) {
    return write(F(" ")).write(key).write(F(": ")).write(val);
  }

  Console Console::section(const String& msg) {
    return write(F(" ")).write(msg);
  }

  Console Console::write(const String& msg) {
    #ifdef VICTOR_DEBUG
      Serial.print(msg);
    #endif
    return console;
  }

  Console Console::write(unsigned long msg) {
    #ifdef VICTOR_DEBUG
      Serial.print(msg);
    #endif
    return console;
  }

  Console Console::newline() {
    #ifdef VICTOR_DEBUG
      Serial.println();
    #endif
    return console;
  }

  // global
  Console console;

} // namespace Victor
