#include "BinaryIO.h"

namespace Victor::Components {

  BinaryIO::BinaryIO() {
    // button
    const auto buttonJson = new PinStorage("/button.json");
    const auto buttonPin = buttonJson->load();
    if (buttonPin != nullptr && buttonPin->enable) {
      _button = new ActionButtonInput(buttonPin);
      _button->onAction = [&](ButtonAction action) {
        if (onButtonAction != nullptr) {
          onButtonAction(action);
        }
      };
    }
    // output
    const auto outputJson = new PinStorage("/output.json");
    const auto outputPin = outputJson->load();
    if (outputPin != nullptr && outputPin->enable) {
      _output = new DigitalOutput(outputPin);
    }
    // output2
    const auto outputJson2 = new PinStorage("/output2.json");
    const auto outputPin2 = outputJson2->load();
    if (outputPin2 != nullptr && outputPin2->enable) {
      _output2 = new DigitalOutput(outputPin2);
    }
    // state
    _stateStorage = new DigitalStateStorage("/state.json");
    const auto state = _stateStorage->load();
    if (state != nullptr) {
      const auto savedState = state->save
        ? state->currentValue
        : state->initialValue;
      setOutputState(savedState);
    }
  }

  BinaryIO::~BinaryIO() {
    if (_button != nullptr) {
      delete _button;
      _button = nullptr;
    }
    if (_output != nullptr) {
      delete _output;
      _output = nullptr;
    }
    if (_output2 != nullptr) {
      delete _output2;
      _output2 = nullptr;
    }
    if (_stateStorage != nullptr) {
      free(_stateStorage);
      _stateStorage = nullptr;
    }
  }

  void BinaryIO::loop() {
    if (_button != nullptr) {
      _button->loop();
    }
  }

  bool BinaryIO::getOutputState() {
    // read actual output state prior
    if (_output != nullptr) {
      return _output->lastValue();
    }
    // read saved output state secondly
    const auto state = _stateStorage->load();
    if (state != nullptr) {
      return state->save
        ? state->currentValue
        : state->initialValue;
    }
    // default
    return false;
  }

  void BinaryIO::setOutputState(bool value) {
    if (_output != nullptr) {
      _output->setValue(value);
    }
    if (_output2 != nullptr) {
      _output2->setValue(value);
    }
    // save state
    auto state = _stateStorage->load();
    if (
      state != nullptr &&
      state->save &&
      state->currentValue != value
    ) {
      state->currentValue = value;
      _stateStorage->save(state);
    }
  }

} // namespace Victor::Components
