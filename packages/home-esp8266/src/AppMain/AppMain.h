#ifndef AppMain_h
#define AppMain_h

#include "Console.h"
#include "BuiltinLed.h"
#include "VictorOTA.h"
#include "VictorWifi.h"

#if VICTOR_FEATURES_WEB
  #include "VictorWeb.h"
#endif

#if VICTOR_FEATURES_RADIO
  #include "VictorRadio.h"
#endif

namespace Victor::Components {

  class AppMain {
   public:
    AppMain();
    ~AppMain();
    void setup();
    void loop(int8_t sleepMode = 0);
    #if VICTOR_FEATURES_WEB
      VictorWeb* webPortal = nullptr;
    #endif
    #if VICTOR_FEATURES_RADIO
      VictorRadio* radioPortal = nullptr;
    #endif

   private:
    uint16_t _sleepMillis = 0;
  };

} // namespace Victor::Components

#endif // AppMain_h
