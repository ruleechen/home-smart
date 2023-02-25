#ifndef BinaryStorage_h
#define BinaryStorage_h

#include "FileStorage.h"

namespace Victor::Components {

  struct BinaryConfig {
    // input pin
    // 0~127 = gpio
    //    -1 = disabled
    int8_t inputPin = -1; // (-128~127)
    // 0 = LOW
    // 1 = HIGH
    uint8_t inputTrueValue = 0; // (0~255) LOW

    // output pin
    // 0~127 = gpio
    //    -1 = disabled
    int8_t outputPin = -1; // (-128~127)
    // 0 = LOW
    // 1 = HIGH
    uint8_t outputTrueValue = 0; // (0~255) LOW

    // output2 pin
    // 0~127 = gpio
    //    -1 = disabled
    int8_t output2Pin = -1; // (-128~127)
    // 0 = LOW
    // 1 = HIGH
    uint8_t output2TrueValue = 0; // (0~255) LOW

    // save output state or not
    bool saveOutput = false;
    // output state saved
    bool outputIsOn = false;
  };

  class BinaryStorage : public FileStorage<BinaryConfig> {
   public:
    BinaryStorage(const char* filePath);

   protected:
    void _serializeTo(const BinaryConfig* model, DynamicJsonDocument& doc) override;
    void _deserializeFrom(BinaryConfig* model, const DynamicJsonDocument& doc) override;
  };

} // namespace Victor::Components

#endif // BinaryStorage_h
