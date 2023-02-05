#include "OccupancySensor.h"

namespace Victor::Components {

  OccupancySensor::OccupancySensor() {
    const auto setting = occupancyStorage.load();
    _input = new DigitalInput(setting->sensorPin, setting->sensorTrueValue);
    _debounce = new IntervalOver(setting->debounce);
    _currentState = readState();
    // register interrupt
    attachInterrupt(digitalPinToInterrupt(setting->sensorPin), _interruptHandler, CHANGE);
  }

  OccupancySensor::~OccupancySensor() {
    if (_input != nullptr) {
      delete _input;
      _input = nullptr;
    }
    if (_debounce != nullptr) {
      free(_debounce);
      _debounce = nullptr;
    }
    onStateChange = nullptr;
    // unregister interrupt
    // detachInterrupt(digitalPinToInterrupt(1));
  }

  void OccupancySensor::loop() {
    if (_hasChanges) {
      _hasChanges = false;
      const auto now = millis();
      if (_debounce->isOver(now)) {
        const auto state = readState();
        if (state != _currentState) {
          _currentState = state;
          _debounce->start(now);
          if (onStateChange != nullptr) {
            onStateChange(state);
          }
        }
      }
    }
  }

  bool OccupancySensor::readState() {
    return _input->getValue();
  }

  volatile bool OccupancySensor::_hasChanges = false;
  void IRAM_ATTR OccupancySensor::_interruptHandler() {
    _hasChanges = true;
  }

} // namespace Victor::Components
