#ifndef I2cSetting_h
#define I2cSetting_h

#include "Pin/Storage/PinConfig.h"

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
    // optional
    PinConfig* chipSelect = nullptr;
  };

} // namespace Victor::Components

#endif // I2cSetting_h
