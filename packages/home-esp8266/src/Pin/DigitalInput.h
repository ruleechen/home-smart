#ifndef DigitalInput_h
#define DigitalInput_h

#include <Arduino.h>
#include "Pin/PinModels.h"

namespace Victor::Components {
  class DigitalInput {
   public:
    DigitalInput(PinConfig* config, uint8_t mode = INPUT_PULLUP);
    bool getValue();

   private:
    PinConfig* _config;
  };

} // namespace Victor::Components

#endif // DigitalInput_h
