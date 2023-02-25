#ifndef PinModels_h
#define PinModels_h

namespace Victor::Components {

  struct PinConfig {
    // pin
    // 0~127 = gpio
    //    -1 = disabled
    int8_t pin = -1; // (-128~127)

    // 0 = LOW
    // 1 = HIGH
    uint8_t trueValue = 0; // (0~255) LOW

    // is enable
    bool enable = true;
  };

} // namespace Victor::Components

#endif // PinModels_h
