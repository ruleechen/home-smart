#include "ActionButtonInput.h"

namespace Victor::Components {

  ActionButtonInput::ActionButtonInput(PinConfig* config) {
    _input = new DigitalInput(config);
    _button = new ActionButton(_input->getValue());
    _button->onAction = [&](ButtonAction action) {
      if (onAction != nullptr) {
        onAction(action);
      }
    };
  }

  ActionButtonInput::~ActionButtonInput() {
    onAction = nullptr;
    if (_input != nullptr) {
      delete _input;
      _input = nullptr;
    }
    if (_button != nullptr) {
      delete _button;
      _button = nullptr;
    }
  }

  void ActionButtonInput::loop() {
    _button->update(_input->getValue());
  }

} // namespace Victor::Components
