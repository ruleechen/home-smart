#ifndef AppStorage_h
#define AppStorage_h

#include "FileStorage.h"
#include "Pin/PinModels.h"

namespace Victor::Components {

  struct AppSetting {
    String name;
    String brand;
    // (0 ~ 65535) for light sleep mode
    // 0         = disabled
    // 1 ~ 65535 = delay ms
    uint16_t sleepMillis = 20;
  };

  class AppStorage : public FileStorage<AppSetting> {
   public:
    AppStorage(const char* filePath = "/app.json");

   protected:
    void _serializeTo(const AppSetting* model, DynamicJsonDocument& doc) override;
    void _deserializeFrom(AppSetting* model, const DynamicJsonDocument& doc) override;
  };

  // global
  extern AppStorage appStorage;

} // namespace Victor::Components

#endif // AppStorage_h
