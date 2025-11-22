#ifndef WiFiSetting_h
#define WiFiSetting_h

#include <Arduino.h>

namespace Victor::Components {

  struct WiFiSetting {
    /*
    * Credential for connect to an AP
    */
    String ssid;
    String pswd;

    /*
    * default true
    * Auto enable light sleep mode when station got an IP
    * Auto disable light sleep mode when station is disconnected
    */
    bool autoMode = true;

    // When listenInterval is set to 1..10, in LIGHT or MODEM mode, station wakes up every (DTIM-interval * listenInterval).
    // This saves power but station interface may miss broadcast data.
    // 1~10 = listenInterval
    //    0 = disabled
    uint8_t dtimMultiplier = 0; // (1~10)
  };

} // namespace Victor::Components

#endif // WiFiSetting_h
