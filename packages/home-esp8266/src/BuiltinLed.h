#ifndef BuiltinLed_h
#define BuiltinLed_h

#include <Arduino.h>
#include <Ticker.h>
#include "Pin/Storage/PinStorage.h"
#include "Pin/DigitalOutput.h"

#ifndef VICTOR_BUILTIN_LED_FLASH
#define VICTOR_BUILTIN_LED_FLASH 10
#endif

#ifndef VICTOR_BUILTIN_LED_TWINKLE
#define VICTOR_BUILTIN_LED_TWINKLE 500
#endif

namespace Victor::Components {
  class BuiltinLed {
   public:
    BuiltinLed();
    ~BuiltinLed();
    void setup(const char* configFile = "/led.json");
    void turnOn();
    void turnOff();
    void toggle();
    void flash(uint16_t duration = VICTOR_BUILTIN_LED_FLASH);
    void twinkle(uint16_t duration = VICTOR_BUILTIN_LED_TWINKLE);
    void stop();

   private:
    Ticker* _ticker = nullptr;
    DigitalOutput* _outputPin = nullptr;
  };

  // global
  extern BuiltinLed builtinLed;

} // namespace Victor::Components

#endif // BuiltinLed_h
