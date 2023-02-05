#ifndef DoorStorage_h
#define DoorStorage_h

#include <FileStorage.h>
#include "DoorModels.h"

namespace Victor::Components {
  class DoorStorage : public FileStorage<DoorSetting> {
   public:
    DoorStorage(const char* filePath = "/door.json");

   protected:
    void _serializeTo(const DoorSetting* model, DynamicJsonDocument& doc) override;
    void _deserializeFrom(DoorSetting* model, const DynamicJsonDocument& doc) override;
  };

  // global
  extern DoorStorage doorStorage;

} // namespace Victor::Components

#endif // DoorStorage_h
