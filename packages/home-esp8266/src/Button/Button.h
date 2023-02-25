#ifndef Button_h
#define Button_h

#include <functional>
#include <Arduino.h>

namespace Victor::Components {

  template <typename TState>
  class Button {
   public:
    Button(TState initState);
    virtual ~Button();
    // state
    virtual void update(TState state, unsigned long timestamp = 0);
    virtual bool isEqual(TState state);
    virtual bool isPressed(TState state);
    // events
    typedef std::function<void(unsigned long timespan)> TEventHandler;
    TEventHandler onPressed = nullptr;
    TEventHandler onReleased = nullptr;

   protected:
    TState _state;
    unsigned long _lastPressed = 0;
  };

  template <typename TState>
  Button<TState>::Button(TState initState) {
    _state = initState;
  }

  template <typename TState>
  Button<TState>::~Button() {
    onPressed = nullptr;
    onReleased = nullptr;
  }

  template <typename TState>
  void Button<TState>::update(TState state, unsigned long timestamp) {
    if (!isEqual(state)) {
      _state = state;
      const auto now = timestamp > 0 ? timestamp : millis();
      const auto timespan = now - _lastPressed;
      if (isPressed(_state)) {
        _lastPressed = now;
        if (onPressed != nullptr) {
          onPressed(timespan);
        }
      } else {
        if (onReleased != nullptr) {
          onReleased(timespan);
        }
      }
    }
  }

  template <typename TState>
  bool Button<TState>::isEqual(TState state) {
    return state == _state;
  }

  template <typename TState>
  bool Button<TState>::isPressed(TState state) {
    return false;
  }

} // namespace Victor::Components

#endif // Button_h
