#include "DigitalInput.h"

namespace Victor::Components {

  DigitalInput::DigitalInput(PinConfig* config, uint8_t mode) {
    _config = config;
    pinMode(_config->pin, mode);
  }

  bool DigitalInput::getValue() {
    const auto value = digitalRead(_config->pin);
    return value == _config->effectiveValue;
  }

} // namespace Victor::Components
