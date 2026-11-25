#ifndef DigitalStateStorage_h
#define DigitalStateStorage_h

#include "DigitalStateConfig.h"
#include "Storage/FileStorage.h"

namespace Victor::Components {

  class DigitalStateStorage : public FileStorage<DigitalStateConfig> {
   public:
    DigitalStateStorage(const char* filePath);

   protected:
    void _serialize(const DigitalStateConfig* model, DynamicJsonDocument& doc) override;
    void _deserialize(DigitalStateConfig* model, const DynamicJsonDocument& doc) override;
  };

} // namespace Victor::Components

#endif // DigitalStateStorage_h
