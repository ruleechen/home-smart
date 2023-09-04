#ifndef AppStorage_h
#define AppStorage_h

#include "Storage/FileStorage.h"
#include "AppSetting.h"

namespace Victor::Components {

  class AppStorage : public FileStorage<AppSetting> {
   public:
    AppStorage(const char* filePath = "/app.json");

   protected:
    void _serialize(const AppSetting* model, DynamicJsonDocument& doc) override;
    void _deserialize(AppSetting* model, const DynamicJsonDocument& doc) override;
  };

  // global
  extern AppStorage appStorage;

} // namespace Victor::Components

#endif // AppStorage_h
