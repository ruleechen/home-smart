#ifndef WiFiStorage_h
#define WiFiStorage_h

#include "Storage/FileStorage.h"
#include "WiFiSetting.h"

namespace Victor::Components {

  class WiFiStorage : public FileStorage<WiFiSetting> {
   public:
    WiFiStorage(const char* filePath = "/wifi.json");

   protected:
    void _serialize(const WiFiSetting* model, DynamicJsonDocument& doc) override;
    void _deserialize(WiFiSetting* model, const DynamicJsonDocument& doc) override;
  };

} // namespace Victor::Components

#endif // WiFiStorage_h
