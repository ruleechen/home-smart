#include "ActionButton.h"

namespace Victor::Components {

  ActionButton::ActionButton(bool initState) : Button(initState) {
    onPressed = [&](const unsigned long timespan) {
      if (onAction != nullptr && timespan > VICTOR_ACTION_BUTTON_DEBOUNCE) {
        onAction(BUTTON_ACTION_PRESSED);
        if (
          VICTOR_ACTION_BUTTON_DOUBLE_PRESS_MIN < timespan &&
          timespan <= VICTOR_ACTION_BUTTON_DOUBLE_PRESS_MAX
        ) {
          onAction(BUTTON_ACTION_DOUBLE_PRESSED);
        }
      }
    };
    onReleased = [&](const unsigned long timespan) {
      if (onAction != nullptr && timespan > VICTOR_ACTION_BUTTON_DEBOUNCE) {
        onAction(BUTTON_ACTION_RELEASED);
        if (timespan >= VICTOR_ACTION_BUTTON_PRESS_HOLD_L2) {
          onAction(BUTTON_ACTION_PRESSED_HOLD_L2);
        } else if (timespan >= VICTOR_ACTION_BUTTON_PRESS_HOLD_L1) {
          onAction(BUTTON_ACTION_PRESSED_HOLD_L1);
        }
      }
    };
  }

  ActionButton::~ActionButton() {
    onAction = nullptr;
  }

  bool ActionButton::isPressed(bool state) {
    return state == true;
  }

} // namespace Victor::Components
