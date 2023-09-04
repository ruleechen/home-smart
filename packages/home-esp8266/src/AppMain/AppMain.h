#ifndef AppMain_h
#define AppMain_h

#include "Console.h"
#include "BuiltinLed.h"
#include "OTA/VictorOTA.h"
#include "WiFi/VictorWiFi.h"
#include "Storage/AppStorage.h"
#include "Timer/IntervalOverAuto.h"

#if VICTOR_FEATURES_WEB
  #include "Web/VictorWeb.h"
#endif

#if VICTOR_FEATURES_RADIO
  #include "Radio/VictorRadio.h"
#endif

namespace Victor::Components {

  class AppMain {
   public:
    AppMain();
    ~AppMain();
    void setup();
    void loop(int8_t sleepMode = 0);
    typedef std::function<void()> THeartbeatHandler;
    THeartbeatHandler onHeartbeat = nullptr;
    #if VICTOR_FEATURES_WEB
      VictorWeb* webPortal = nullptr;
    #endif
    #if VICTOR_FEATURES_RADIO
      VictorRadio* radioPortal = nullptr;
    #endif

   private:
    uint16_t _sleepMillis = 0;
    IntervalOverAuto* _heartbeat = nullptr;
  };

} // namespace Victor::Components

#endif // AppMain_h
