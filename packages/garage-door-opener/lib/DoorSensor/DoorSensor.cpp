#include "DoorSensor.h"

namespace Victor::Components {

  DoorSensor::DoorSensor() {
    // open sensor
    const auto openJson = new PinStorage("/doorOpen.json");
    const auto openPin = openJson->load();
    if (openPin->enable) {
      _openSensor = new DigitalInput(openPin);
      attachInterrupt(digitalPinToInterrupt(openPin->pin), _interruptHandler, CHANGE);
    }
    // closed sensor
    const auto closedJson = new PinStorage("/doorClosed.json");
    const auto closedPin = closedJson->load();
    if (closedPin->enable) {
      _closedSensor = new DigitalInput(closedPin);
      attachInterrupt(digitalPinToInterrupt(closedPin->pin), _interruptHandler, CHANGE);
    }
    // other settings
    const auto setting = doorStorage.load();
    _debounce = new IntervalOver(setting->debounce);
    _currentState = readState();
  }

  DoorSensor::~DoorSensor() {
    if (_openSensor != nullptr) {
      delete _openSensor;
      _openSensor = nullptr;
    }
    if (_closedSensor != nullptr) {
      delete _closedSensor;
      _closedSensor = nullptr;
    }
    if (_debounce != nullptr) {
      free(_debounce);
      _debounce = nullptr;
    }
    onStateChange = nullptr;
    // unregister interrupt
    // detachInterrupt(digitalPinToInterrupt(1));
    // detachInterrupt(digitalPinToInterrupt(2));
  }

  void DoorSensor::loop() {
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

  CurrentDoorState DoorSensor::readState() {
    auto state = _currentState;
    if (_openSensor->getValue()) {
      state = CURRENT_DOOR_STATE_OPEN;
    } else if (_closedSensor->getValue()) {
      state = CURRENT_DOOR_STATE_CLOSED;
    } else {
      if (_currentState == CURRENT_DOOR_STATE_OPEN) {
        state = CURRENT_DOOR_STATE_CLOSING;
      } else if (_currentState == CURRENT_DOOR_STATE_CLOSED) {
        state = CURRENT_DOOR_STATE_OPENING;
      }
    }
    return state;
  }

  volatile bool DoorSensor::_hasChanges = false;
  void IRAM_ATTR DoorSensor::_interruptHandler() {
    _hasChanges = true;
  }

} // namespace Victor::Components
