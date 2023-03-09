#include "DigitalSensor.h"

namespace Victor::Components {

  DigitalSensor::DigitalSensor(const char* pinJson) : StateSensor(false) {
    const auto storage = new PinStorage(pinJson);
    const auto config = storage->load();
    if (config->enable) {
      _input = new DigitalInput(config);
      attachInterrupt(digitalPinToInterrupt(config->pin), _interruptHandler, CHANGE);
      setDebounce(config->debounce);
    }
  }

  DigitalSensor::~DigitalSensor() {
    if (_input != nullptr) {
      delete _input;
      _input = nullptr;
    }
    // unregister interrupt
    // detachInterrupt(digitalPinToInterrupt(1));
  }

  bool DigitalSensor::readState() {
    return _input != nullptr
      ? _input->getValue()
      : false;
  }

  void IRAM_ATTR DigitalSensor::_interruptHandler() {
    reportChange();
  }

} // namespace Victor::Components
