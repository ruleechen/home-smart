#ifndef I2cStorage_h
#define I2cStorage_h

#include "Storage/FileStorage.h"
#include "I2cSetting.h"

namespace Victor::Components {

  class I2cStorage : public FileStorage<I2cSetting> {
   public:
    I2cStorage(const char* filePath = "/i2c.json");

   protected:
    void _serialize(const I2cSetting* model, DynamicJsonDocument& doc) override;
    void _deserialize(I2cSetting* model, const DynamicJsonDocument& doc) override;
  };

} // namespace Victor::Components

#endif // I2cStorage_h
