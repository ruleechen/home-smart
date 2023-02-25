#ifndef I2cStorage_h
#define I2cStorage_h

#include "FileStorage.h"

namespace Victor::Components {

  // https://zhuanlan.zhihu.com/p/137568249
  struct I2cSetting {
    // pin I2C-SDA
    // Inter-Integrated Circuit - Serial Data (I2C-SDA)
    uint8_t sdaPin = 4; // (0~255)
    // pin I2C-SCL
    // Inter-Integrated Circuit - Serial Clock (I2C-SCL)
    uint8_t sclPin = 5; // (0~255)

    // chip select signal for power on/off i2c bus
    // 0~127 = gpio
    //    -1 = disabled
    int8_t enablePin = -1; // (-128~127)
    // 0 = LOW
    // 1 = HIGH
    uint8_t enableTrueValue = 0; // (0~255)
  };

  class I2cStorage : public FileStorage<I2cSetting> {
   public:
    I2cStorage(const char* filePath = "/i2c.json");

   protected:
    void _serializeTo(const I2cSetting* model, DynamicJsonDocument& doc) override;
    void _deserializeFrom(I2cSetting* model, const DynamicJsonDocument& doc) override;
  };

} // namespace Victor::Components

#endif // I2cStorage_h
