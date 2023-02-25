#include "DigitalOutput.h"

namespace Victor::Components {

  DigitalOutput::DigitalOutput(uint8_t pin, uint8_t trueValue, uint8_t mode) {
    _pin = pin;
    _trueValue = trueValue;
    pinMode(_pin, mode);
  }

  void DigitalOutput::setValue(bool value) {
    const auto falseValue = _trueValue == LOW ? HIGH : LOW;
    digitalWrite(_pin, value ? _trueValue : falseValue);
    _lastValue = value;
  }

  bool DigitalOutput::lastValue() const {
    return _lastValue;
  }

} // namespace Victor::Components
