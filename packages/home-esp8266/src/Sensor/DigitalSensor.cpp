#include "DigitalSensor.h"

namespace Victor::Components {

  DigitalSensor::DigitalSensor(const char* pinJson) : StateSensor(false) {
    const auto storage = new PinStorage(pinJson);
    const auto config = storage->load();
    if (config->enable) {
      _input = new DigitalInput(config);
      setDebounce(config->debounce);
      setHeartbeat(config->heartbeat);
      if (config->interruptMode > 0) {
        _changed = STATE_CHANGE_NO;
        const auto interruptPin = digitalPinToInterrupt(config->pin);
        attachInterrupt(interruptPin, _interruptHandler, config->interruptMode);
      }
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
    _changed = STATE_CHANGE_YES;
  }

} // namespace Victor::Components
