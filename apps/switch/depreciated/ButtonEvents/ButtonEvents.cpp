#include "ButtonEvents.h"

namespace Victor::Events {

  ButtonEvents::ButtonEvents(TInputHandler getPressed) {
    _getPressed = getPressed;
  }

  ButtonEvents::~ButtonEvents() {
    _getPressed = NULL;
  }

  void ButtonEvents::loop() {
    const auto state = _loadState();
    if (state != _lastState) {
      _lastState = state;
      if (onClick) {
        onClick(state);
      }
    }
  }

  int ButtonEvents::_loadState() {
    if (_buttonState == AWAIT_PRESS) {
      _clicks = 0;
      if (_getPressed()) {
        _buttonState = DEBOUNCE_PRESS;
        _eventTime = millis();
      }
    }

    else if (_buttonState == DEBOUNCE_PRESS) {
      if ((millis() - _eventTime) > DEBOUNCE_PRESS_TIME) {
        _buttonState = AWAIT_RELEASE;
        _eventTime = millis();
      }
    }

    else if (_buttonState == AWAIT_RELEASE) {
      if (!_getPressed()) {
        if ((millis() - _eventTime) > HOLD_TIME) {
          _clicks = -1;
        }
        _buttonState = DEBOUNCE_RELEASE;
        _eventTime = millis();
      }
    }

    else if (_buttonState == DEBOUNCE_RELEASE) {
      if ((millis() - _eventTime) > DEBOUNCE_RELEASE_TIME) {
        if (_clicks < 0) {
          _buttonState = AWAIT_PRESS;
          return -1;
        }
        _clicks += 1;
        _buttonState = AWAIT_MULTI_PRESS;
        _eventTime = millis();
      }
    }

    else { // (_buttonState == AWAIT_MULTI_PRESS)
      if (_getPressed()) {
        _buttonState = DEBOUNCE_PRESS;
        _eventTime = millis();
      } else if ((millis() - _eventTime) > MULTI_CLICK_TIME) {
        _buttonState = AWAIT_PRESS;
        return _clicks;
      }
    }

    return 0;
  }

} // namespace Victor::Events
