#include "DigitalOutput.h"

namespace Victor::Components {

  DigitalOutput::DigitalOutput(PinConfig* config, uint8_t mode) {
    _config = config;
    pinMode(_config->pin, mode);
  }

  void DigitalOutput::setValue(bool value) {
    const auto falseValue = _config->trueValue == LOW ? HIGH : LOW;
    digitalWrite(_config->pin, value ? _config->trueValue : falseValue);
    _lastValue = value;
  }

  bool DigitalOutput::lastValue() const {
    return _lastValue;
  }

} // namespace Victor::Components
