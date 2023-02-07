#include "LeakSensor.h"

namespace Victor::Components {

  LeakSensor::LeakSensor() {
    const auto setting = leakStorage.load();
    _input = new DigitalInput(setting->sensorPin, setting->sensorTrueValue);
    _loop = new IntervalOverAuto(setting->loop);
    _debounce = new IntervalOver(setting->debounce);
    _currentState = readState();
    // register interrupt
    attachInterrupt(digitalPinToInterrupt(setting->sensorPin), _interruptHandler, CHANGE);
  }

  LeakSensor::~LeakSensor() {
    if (_input != nullptr) {
      delete _input;
      _input = nullptr;
    }
    if (_loop != nullptr) {
      free(_loop);
      _loop = nullptr;
    }
    if (_debounce != nullptr) {
      free(_debounce);
      _debounce = nullptr;
    }
    onLoop = nullptr;
    onStateChange = nullptr;
    // unregister interrupt
    // detachInterrupt(digitalPinToInterrupt(1));
  }

  void LeakSensor::loop() {
    const auto now = millis();
    if (_hasChanges || _loop->isOver(now)) {
      _hasChanges = false;
      if (onLoop != nullptr) {
        const auto analog = readAnalog();
        onLoop(analog);
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
