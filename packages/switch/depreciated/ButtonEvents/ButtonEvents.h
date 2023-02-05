#ifndef ButtonEvents_h
#define ButtonEvents_h

#include <functional>
#include <Arduino.h>
#include "Commons.h"

// Migrate from
// https://www.sigmdel.ca/michel/program/esp8266/arduino/switch_debouncing_en.html

#define DEBOUNCE_PRESS_TIME   15
#define DEBOUNCE_RELEASE_TIME 30
#define MULTI_CLICK_TIME     400
#define HOLD_TIME           2000

namespace Victor::Events {
  class ButtonEvents {
    typedef std::function<bool()> TInputHandler;
    typedef std::function<void(int)> TClickHandler;

   public:
    ButtonEvents(TInputHandler getPressed);
    ~ButtonEvents();
    TClickHandler onClick;
    void loop();

   private:
    TInputHandler _getPressed;
    // states
    int _clicks = 0;
    unsigned long _eventTime = 0;
    ButtonState _buttonState = AWAIT_PRESS;
    // status, number of clicks since last update
    // -1 = button held, 0 = button up, 1, 2, ... number of times button clicked
    int _loadState();
    int _lastState = 0;
  };
} // namespace Victor::Events

#endif // ButtonEvents_h
