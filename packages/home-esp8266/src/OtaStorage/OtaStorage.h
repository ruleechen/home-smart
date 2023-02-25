#ifndef OtaStorage_h
#define OtaStorage_h

#include "FileStorage.h"

namespace Victor::Components {

  struct OtaSetting {
    // remote uri for query new Firmware/FileSystem image
    String remote;
  };

  class OtaStorage : public FileStorage<OtaSetting> {
   public:
    OtaStorage(const char* filePath = "/ota.json");

   protected:
    void _serializeTo(const OtaSetting* model, DynamicJsonDocument& doc) override;
    void _deserializeFrom(OtaSetting* model, const DynamicJsonDocument& doc) override;
  };

} // namespace Victor::Components

#endif // OtaStorage_h
