#ifndef DigitalOutput_h
#define DigitalOutput_h

#include <Arduino.h>
#include "Pin/PinModels.h"

namespace Victor::Components {
  class DigitalOutput {
   public:
    DigitalOutput(PinConfig* config, uint8_t mode = OUTPUT);
    void setValue(bool value);
    bool lastValue() const;

   private:
    PinConfig* _config;
    bool _lastValue = false;
  };

} // namespace Victor::Components

#endif // DigitalOutput_h
