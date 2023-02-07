#ifndef LeakSensor_h
#define LeakSensor_h

#include <Arduino.h>
#include <DigitalInput.h>
#include <Timer/IntervalOver.h>
#include <Timer/IntervalOverAuto.h>
#include "LeakStorage.h"

namespace Victor::Components {
  class LeakSensor {
   public:
    LeakSensor();
    ~LeakSensor();
    void loop();
    bool readState();
    int readAnalog();
    // events
    typedef std::function<void(const bool state)> TStateHandler;
    TStateHandler onStateChange = nullptr;
    typedef std::function<void(const int analog)> TLoopHandler;
    TLoopHandler onLoop = nullptr;

   private:
    DigitalInput* _input = nullptr;
    IntervalOver* _loop = nullptr;
    IntervalOver* _debounce = nullptr;
    bool _currentState = false;
    // interrupt
    volatile static bool _hasChanges;
    static void IRAM_ATTR _interruptHandler();
  };

} // namespace Victor::Components

#endif // LeakSensor_h
