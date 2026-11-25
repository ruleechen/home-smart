#ifndef HalInterrupt_h
#define HalInterrupt_h

#include <Arduino.h>
#include <functional>

namespace Victor::HAL {

struct InterruptInfo {
    uint8_t pin = 0;
    uint8_t value = 0;
    uint32_t micro = 0;
};

using InterruptCallback = std::function<void(const InterruptInfo)>;

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
  void attachScheduledInterrupt(uint8_t pin, InterruptCallback cb, int mode);
  void processScheduledInterrupts();
#endif

} // namespace Victor::HAL

#endif // HalInterrupt_h
