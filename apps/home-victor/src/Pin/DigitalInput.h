#ifndef DigitalInput_h
#define DigitalInput_h

#include <Arduino.h>
#include "Storage/PinStorage/PinConfig.h"

namespace Victor::Components {
  class DigitalInput {
   public:
    DigitalInput(PinConfig* config, uint8_t mode = INPUT_PULLUP);
    bool getValue();

   private:
    PinConfig* _config = nullptr;
  };

} // namespace Victor::Components

#endif // DigitalInput_h
