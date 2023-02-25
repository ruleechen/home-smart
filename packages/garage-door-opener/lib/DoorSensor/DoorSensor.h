#ifndef DoorSensor_h
#define DoorSensor_h

#include <Arduino.h>
#include <Pin/DigitalInput.h>
#include <Timer/IntervalOver.h>
#include "DoorStorage.h"

namespace Victor::Components {
  class DoorSensor {
   public:
    DoorSensor();
    ~DoorSensor();
    void loop();
    CurrentDoorState readState();
    // events
    typedef std::function<void(const CurrentDoorState currentState)> TStateHandler;
    TStateHandler onStateChange = nullptr;

   private:
    DigitalInput* _openSensor = nullptr;
    DigitalInput* _closedSensor = nullptr;
    IntervalOver* _debounce = nullptr;
    CurrentDoorState _currentState = CURRENT_DOOR_STATE_STOPPED;
    // interrupt
    volatile static bool _hasChanges;
    static void IRAM_ATTR _interruptHandler();
  };

} // namespace Victor::Components

#endif // DoorSensor_h
