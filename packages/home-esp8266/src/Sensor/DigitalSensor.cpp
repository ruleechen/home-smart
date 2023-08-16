#include "DigitalSensor.h"

namespace Victor::Components {

  DigitalSensor::DigitalSensor(const char* pinJson) : StateSensor(false) {
    const auto storage = new PinStorage(pinJson);
    const auto config = storage->load();
    if (config != nullptr && config->enable) {
      _input = new DigitalInput(config);
      setDebounce(config->debounce);
      if (config->interruptMode > 0) {
        _changed = STATE_CHANGE_NO;
        const auto interruptPin = digitalPinToInterrupt(config->pin);
        attachScheduledInterrupt(interruptPin, [&](const InterruptInfo info) {
          this->_interruptHandler(info);
        }, config->interruptMode);
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

  void DigitalSensor::_interruptHandler(const InterruptInfo info) {
    _changed = STATE_CHANGE_YES;
    console.log()
      .bracket(F("interrupt"))
      .section(F("pin"), String(info.pin))
      .section(F("value"), String(info.value));
  }

} // namespace Victor::Components
