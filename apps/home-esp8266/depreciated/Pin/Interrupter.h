#ifndef Interrupter_h
#define Interrupter_h

#include <map>
#include <vector>
#include <Arduino.h>

namespace Victor::Components {

  typedef void(*StaticInterruptHandler)();

  void _emitInterrupt(uint8_t index);

  struct InterrupterConfig {
    uint8_t index = 0;
    StaticInterruptHandler handler = nullptr;
  };

  class Interrupter {
   public:
    Interrupter();
    ~Interrupter();
    StaticInterruptHandler getHandler();
    void _emit();
    typedef std::function<void(const uint8_t index)> TInterruptHandler;
    TInterruptHandler onInterrupt = nullptr;

   private:
    InterrupterConfig* _config = nullptr;
  };

} // namespace Victor::Components

#endif // Interrupter_h
