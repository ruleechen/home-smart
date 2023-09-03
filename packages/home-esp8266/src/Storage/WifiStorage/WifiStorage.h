#ifndef WifiStorage_h
#define WifiStorage_h

#include "FileStorage.h"
#include "WifiSetting.h"

namespace Victor::Components {

  class WifiStorage : public FileStorage<WifiSetting> {
   public:
    WifiStorage(const char* filePath = "/wifi.json");

   protected:
    void _serialize(const WifiSetting* model, DynamicJsonDocument& doc) override;
    void _deserialize(WifiSetting* model, const DynamicJsonDocument& doc) override;
  };

} // namespace Victor::Components

#endif // WifiStorage_h
