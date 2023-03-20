#include "DoorSensor.h"

namespace Victor::Components {

  DoorSensor::DoorSensor() : StateSensor(CURRENT_DOOR_STATE_STOPPED) {
    // open sensor
    const auto openJson = new PinStorage("/doorOpen.json");
    const auto openPin = openJson->load();
    if (openPin->enable) {
      _openSensor = new DigitalInput(openPin);
      if (openPin->interruptMode > 0) {
        const auto interruptPin = digitalPinToInterrupt(openPin->pin);
        attachInterrupt(interruptPin, _interruptHandler, openPin->interruptMode);
      }
    }
    // closed sensor
    const auto closedJson = new PinStorage("/doorClosed.json");
    const auto closedPin = closedJson->load();
    if (closedPin->enable) {
      _closedSensor = new DigitalInput(closedPin);
      if (closedPin->interruptMode > 0) {
        const auto interruptPin = digitalPinToInterrupt(closedPin->pin);
        attachInterrupt(interruptPin, _interruptHandler, closedPin->interruptMode);
      }
    }
    // debounce
    const auto setting = doorStorage.load();
    setDebounce(setting->debounce);
    // others
    _state = readState();
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
  }

  CurrentDoorState DoorSensor::readState() {
    auto state = _state;
    if (_openSensor->getValue()) {
      state = CURRENT_DOOR_STATE_OPEN;
    } else if (_closedSensor->getValue()) {
      state = CURRENT_DOOR_STATE_CLOSED;
    } else {
      if (_state == CURRENT_DOOR_STATE_OPEN) {
        state = CURRENT_DOOR_STATE_CLOSING;
      } else if (_state == CURRENT_DOOR_STATE_CLOSED) {
        state = CURRENT_DOOR_STATE_OPENING;
      }
    }
    return state;
  }

  void IRAM_ATTR DoorSensor::_interruptHandler() {
    reportChange();
  }

} // namespace Victor::Components
