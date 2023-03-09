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

    // ms debounce time to avoid fast changes
    // optional, default 0
    uint16_t debounce = 0; // (0~65535)

    // ms heartbeat interval
    // optional, default 0
    uint16_t heartbeat = 0; // (0~65535)

    // is enable
    // optional, default yes
    bool enable = true;
  };

} // namespace Victor::Components

#endif // PinModels_h
