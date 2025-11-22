#ifndef OtaStorage_h
#define OtaStorage_h

#include "Storage/FileStorage.h"
#include "OtaSetting.h"

namespace Victor::Components {

  class OtaStorage : public FileStorage<OtaSetting> {
   public:
    OtaStorage(const char* filePath = "/ota.json");

   protected:
    void _serialize(const OtaSetting* model, DynamicJsonDocument& doc) override;
    void _deserialize(OtaSetting* model, const DynamicJsonDocument& doc) override;
  };

} // namespace Victor::Components

#endif // OtaStorage_h
