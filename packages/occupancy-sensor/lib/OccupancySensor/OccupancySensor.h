#ifndef OccupancySensor_h
#define OccupancySensor_h

#include <Arduino.h>
#include <DigitalInput.h>
#include <Timer/IntervalOver.h>
#include "OccupancyStorage.h"

namespace Victor::Components {
  class OccupancySensor {
   public:
    OccupancySensor();
    ~OccupancySensor();
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

#endif // OccupancySensor_h
