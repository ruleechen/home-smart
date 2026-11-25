#include "HAL/HalInterrupt.h"

#if defined(ARDUINO_ARCH_ESP8266)
  // ESP8266 Arduino core provides ::InterruptInfo and ::attachScheduledInterrupt
  // in FunctionalInterrupt.h. Adapt our InterruptCallback to that signature.
  #include <FunctionalInterrupt.h>

  namespace Victor::HAL {

  void attachScheduledInterrupt(uint8_t pin, InterruptCallback cb, int mode) {
    ::attachScheduledInterrupt(pin, [cb](const ::InterruptInfo info) {
      cb(InterruptInfo{info.pin, info.value, info.micro});
    }, mode);
  }

  void processScheduledInterrupts() {
    // ESP8266's attachScheduledInterrupt dispatches via schedule_function(),
    // which runs callbacks in the main loop automatically.
    // Nothing to do here.
  }

  } // namespace Victor::HAL

#elif defined(ARDUINO_ARCH_ESP32)
  namespace Victor::HAL {
  namespace detail {
    constexpr size_t kMaxPendingInterrupts = 16;
    InterruptInfo pendingBuffer[kMaxPendingInterrupts];
    volatile size_t pendingWrite = 0;
    volatile size_t pendingRead = 0;
    InterruptCallback pinCallbacks[40] = {nullptr};
    // ESP32's attachInterrupt only accepts void (*)(), so we use a
    // module-level variable to know which pin fired the interrupt.
    volatile uint8_t irqPin = 255;

    void IRAM_ATTR onPinInterrupt() {
      if (irqPin >= 40) return;
      InterruptInfo info;
      info.pin = irqPin;
      info.value = digitalRead(irqPin);
      info.micro = micros();

      const auto next = (pendingWrite + 1) % kMaxPendingInterrupts;
      if (next != pendingRead) {
        pendingBuffer[pendingWrite] = info;
        pendingWrite = next;
      }
    }
  } // namespace detail

  void attachScheduledInterrupt(uint8_t pin, InterruptCallback cb, int mode) {
    if (pin >= 40) return;
    detail::pinCallbacks[pin] = cb;
    detail::irqPin = pin;
    ::attachInterrupt(pin, detail::onPinInterrupt, mode);
  }

  void processScheduledInterrupts() {
    while (detail::pendingRead != detail::pendingWrite) {
      const auto info = detail::pendingBuffer[detail::pendingRead];
      detail::pendingRead = (detail::pendingRead + 1) % detail::kMaxPendingInterrupts;
      if (info.pin < 40 && detail::pinCallbacks[info.pin]) {
        detail::pinCallbacks[info.pin](info);
      }
    }
  }
  } // namespace Victor::HAL
#endif
