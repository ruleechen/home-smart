#ifndef WifiStorage_h
#define WifiStorage_h

#include "FileStorage.h"

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

  class WifiStorage : public FileStorage<WifiSetting> {
   public:
    WifiStorage(const char* filePath = "/wifi.json");

   protected:
    void _serializeTo(const WifiSetting* model, DynamicJsonDocument& doc) override;
    void _deserializeFrom(WifiSetting* model, const DynamicJsonDocument& doc) override;
  };

} // namespace Victor::Components

#endif // WifiStorage_h
