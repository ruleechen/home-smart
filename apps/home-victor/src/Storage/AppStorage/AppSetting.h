#ifndef AppSetting_h
#define AppSetting_h

#include <Arduino.h>

namespace Victor::Components {

  struct AppSetting {
    String name;
    String brand;

    // (0 ~ 65535) for light sleep mode
    // 0         = disabled
    // 1 ~ 65535 = delay ms
    uint16_t sleepMillis = 20;

    // ms heartbeat interval
    // optional, default 0
    uint16_t heartbeat = 0; // (0~65535)
  };

} // namespace Victor::Components

#endif // AppSetting_h
