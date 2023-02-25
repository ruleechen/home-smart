#include "LeakSensor.h"

namespace Victor::Components {

  LeakSensor::LeakSensor() {
    const auto setting = leakStorage.load();
    _input = new DigitalInput(setting->sensor);
    _heartbeat = new IntervalOverAuto(setting->heartbeat);
    _debounce = new IntervalOver(setting->debounce);
    _currentState = readState();
    // register interrupt
    attachInterrupt(digitalPinToInterrupt(setting->sensor->pin), _interruptHandler, CHANGE);
  }

  LeakSensor::~LeakSensor() {
    if (_input != nullptr) {
      delete _input;
      _input = nullptr;
    }
    if (_heartbeat != nullptr) {
      free(_heartbeat);
      _heartbeat = nullptr;
    }
    if (_debounce != nullptr) {
      free(_debounce);
      _debounce = nullptr;
    }
    onHeartbeat = nullptr;
    onStateChange = nullptr;
    // unregister interrupt
    // detachInterrupt(digitalPinToInterrupt(1));
  }

  void LeakSensor::loop() {
    const auto now = millis();
    if (_hasChanges || _heartbeat->isOver(now)) {
      _hasChanges = false;
      if (onHeartbeat != nullptr) {
        const auto analog = readAnalog();
        onHeartbeat(analog);
      }
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

  bool LeakSensor::readState() {
    return _input->getValue();
  }

  int LeakSensor::readAnalog() {
    return analogRead(A0);
  }

  volatile bool LeakSensor::_hasChanges = false;
  void IRAM_ATTR LeakSensor::_interruptHandler() {
    _hasChanges = true;
  }

} // namespace Victor::Components
