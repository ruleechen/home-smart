#include "ActionButtonInterrupt.h"

namespace Victor::Components {

  ActionButtonInterrupt::ActionButtonInterrupt(PinConfig* config) : ActionButtonInput(config) {
    _lastInputValue = _input->getValue();
    const auto interruptPin = digitalPinToInterrupt(config->pin);
    attachScheduledInterrupt(interruptPin, [&](const InterruptInfo info) {
      this->_interruptHandler(info);
    }, CHANGE);
  }

  ActionButtonInterrupt::~ActionButtonInterrupt() {
    // detachInterrupt(digitalPinToInterrupt(inputPin));
  }

  void ActionButtonInterrupt::loop() {
    if (_contexts.size() > 1) { // at least 2
      // every loop apply 2 interrupt contexts
      for (size_t i = 0; i + 1 < _contexts.size(); i = i + 2) {
        const auto item1 = _contexts[i];
        const auto item2 = _contexts[i + 1];
        _button->update(item1->inputValue, item1->timestamp);
        _button->update(item2->inputValue, item2->timestamp);
      }
      // reset
      for (auto context : _contexts) { delete context; }
      _contexts.clear();
      _contexts.shrink_to_fit();
    }
  }

  void ActionButtonInterrupt::_interruptHandler(const InterruptInfo info) {
    if (_contexts.size() < VICTOR_DIGITAL_INPUT_MAX_CHANGES) {
      const auto inputValue = _input->getValue();
      if (inputValue != _lastInputValue) {
        _lastInputValue = inputValue;
        _contexts.push_back(new InterruptContext({
          .inputValue = inputValue,
          .timestamp = millis(),
        }));
        console.log()
          .bracket(F("interrupt"))
          .section(F("pin"), String(info.pin))
          .section(F("value"), String(info.value));
      }
    }
  }

} // namespace Victor::Components
