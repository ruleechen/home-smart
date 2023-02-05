#ifndef OnOffEvents_h
#define OnOffEvents_h

#include <functional>
#include <Arduino.h>

namespace Victor::Events {
  class OnOffEvents {
    typedef std::function<void(bool)> TChangeHandler;

   public:
    OnOffEvents(uint8_t inputPin);
    TChangeHandler onChange;
    void loop();

   private:
    uint8_t _inputPin;
    bool _lastState = false;
    unsigned long _lastTime = 0;
  };
} // namespace Victor::Events

#endif // OnOffEvents_h
