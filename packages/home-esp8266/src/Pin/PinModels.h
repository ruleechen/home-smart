#ifndef PinModels_h
#define PinModels_h

namespace Victor::Components {

  struct PinConfig {
    // pin
    // 0~127 = gpio
    //    -1 = disabled
    int8_t pin = -1; // (-128~127)

    /**
     * pin true value
     * LOW  = 0
     * HIGH = 1
     */
    uint8_t trueValue = 0; // (0~255) LOW

    /**
     * pin interrupt mode when input
     * optional, default none with 0
     * RISING    - 0x01 -  1
     * FALLING   - 0x02 -  2
     * CHANGE    - 0x03 -  3
     * ONLOW     - 0x04 -  4
     * ONHIGH    - 0x05 -  5
     * ONLOW_WE  - 0x0C - 12
     * ONHIGH_WE - 0x0D - 13
     */
    uint8_t interruptMode = 0;

    // ms debounce time to avoid fast changes
    // optional, default 0
    uint16_t debounce = 0; // (0~65535)

    // is enable
    // optional, default yes
    bool enable = true;
  };

} // namespace Victor::Components

#endif // PinModels_h
