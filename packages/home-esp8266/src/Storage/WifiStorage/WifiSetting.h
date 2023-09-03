#ifndef WifiSetting_h
#define WifiSetting_h

#include <Arduino.h>

namespace Victor::Components {

  struct WifiSetting {
    String ssid;
    String pswd;
    bool autoMode = true;
    // When listenInterval is set to 1..10, in LIGHT or MODEM mode, station wakes up every (DTIM-interval * listenInterval).
    // This saves power but station interface may miss broadcast data.
    // 1~10 = listenInterval
    //    0 = disabled
    uint8_t dtimMultiplier = 0; // (1~10)
  };

} // namespace Victor::Components

#endif // WifiSetting_h
