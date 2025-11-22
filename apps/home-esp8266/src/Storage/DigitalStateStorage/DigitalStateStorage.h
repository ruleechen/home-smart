#ifndef DigitalStateStorage_h
#define DigitalStateStorage_h

#include "Storage/FileStorage.h"
#include "DigitalStateConfig.h"

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
