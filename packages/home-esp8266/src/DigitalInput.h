#ifndef DigitalInput_h
#define DigitalInput_h

#include <Arduino.h>

namespace Victor::Components {
  class DigitalInput {
   public:
    DigitalInput(uint8_t pin, uint8_t trueValue, uint8_t mode = INPUT_PULLUP);
    bool getValue();

   private:
    uint8_t _pin;
    uint8_t _trueValue;
  };

} // namespace Victor::Components

#endif // DigitalInput_h
