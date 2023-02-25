#include "DigitalInput.h"

namespace Victor::Components {

  DigitalInput::DigitalInput(uint8_t pin, uint8_t trueValue, uint8_t mode) {
    _pin = pin;
    _trueValue = trueValue;
    pinMode(_pin, mode);
  }

  bool DigitalInput::getValue() {
    const auto value = digitalRead(_pin);
    return value == _trueValue;
  }

} // namespace Victor::Components
