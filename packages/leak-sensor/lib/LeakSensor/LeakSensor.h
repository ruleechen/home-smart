#ifndef LeakSensor_h
#define LeakSensor_h

#include <Arduino.h>
#include <DigitalInput.h>
#include <Timer/IntervalOver.h>
#include "LeakStorage.h"

namespace Victor::Components {
  class LeakSensor {
   public:
    LeakSensor();
    ~LeakSensor();
    void loop();
    bool readState();
    // events
    typedef std::function<void(const bool currentState)> TStateHandler;
    TStateHandler onStateChange = nullptr;

   private:
    DigitalInput* _input = nullptr;
    IntervalOver* _debounce = nullptr;
    bool _currentState = false;
    // interrupt
    volatile static bool _hasChanges;
    static void IRAM_ATTR _interruptHandler();
  };

} // namespace Victor::Components

#endif // LeakSensor_h
