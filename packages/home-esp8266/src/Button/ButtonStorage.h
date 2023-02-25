#ifndef ButtonStorage_h
#define ButtonStorage_h

#include "FileStorage.h"

namespace Victor::Components {

  struct ButtonSetting {
    // chip select signal for power on/off i2c bus
    // 0~127 = gpio
    //    -1 = disabled
    int8_t buttonPin = -1; // (-128~127)
    // 0 = LOW
    // 1 = HIGH
    uint8_t buttonTrueValue = 0; // (0~255)
  };

  class ButtonStorage : public FileStorage<ButtonSetting> {
   public:
    ButtonStorage(const char* filePath = "/button.json");

   protected:
    void _serializeTo(const ButtonSetting* model, DynamicJsonDocument& doc) override;
    void _deserializeFrom(ButtonSetting* model, const DynamicJsonDocument& doc) override;
  };

} // namespace Victor::Components

#endif // ButtonStorage_h
