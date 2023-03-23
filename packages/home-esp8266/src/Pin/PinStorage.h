#ifndef PinStorage_h
#define PinStorage_h

#include "FileStorage.h"
#include "PinModels.h"

namespace Victor::Components {
  class PinStorage : public FileStorage<PinConfig> {
   public:
    PinStorage(const char* filePath);

   protected:
    void _serialize(const PinConfig* model, DynamicJsonDocument& doc) override;
    void _deserialize(PinConfig* model, const DynamicJsonDocument& doc) override;
  };

} // namespace Victor::Components

#endif // PinStorage_h
