#ifndef DoorModels_h
#define DoorModels_h

#include <Arduino.h>

namespace Victor::Components {

  // HAP section 9.118; 0 = opened, 1 = closed
  enum TargetDoorState {
    TARGET_DOOR_STATE_OPEN   = 0,
    TARGET_DOOR_STATE_CLOSED = 1,
  };

  // HAP section 9.30; 0 = opened, 1 = closed, 2 = opening, 3 = closing, 4 = stopped not open or closed
  enum CurrentDoorState {
    CURRENT_DOOR_STATE_OPEN    = 0,
    CURRENT_DOOR_STATE_CLOSED  = 1,
    CURRENT_DOOR_STATE_OPENING = 2,
    CURRENT_DOOR_STATE_CLOSING = 3,
    CURRENT_DOOR_STATE_STOPPED = 4,
  };

  enum DoorCommand {
    DOOR_COMMAND_OPEN  = 0,
    DOOR_COMMAND_CLOSE = 1,
    DOOR_COMMAND_STOP  = 2,
  };

  struct DoorSetting {
    // ms debounce time to avoid fast changes
    uint16_t debounce = 0; // (0~65535)

    // auto stop
    //         0 = disabled
    //         1 = enabled
    // 2 ~ 65535 = ms time delay before emit stop command
    uint16_t autoStop = 0; // (0~65535)
  };

} // namespace Victor::Components

#endif // DoorModels_h
