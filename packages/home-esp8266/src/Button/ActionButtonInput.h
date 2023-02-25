#ifndef ActionButtonInput_h
#define ActionButtonInput_h

#include "DigitalInput.h"
#include "Button/ActionButton.h"

namespace Victor::Components {

  class ActionButtonInput {
   public:
    ActionButtonInput(uint8_t inputPin, uint8_t inputTrueValue);
    virtual ~ActionButtonInput();
    virtual void loop();
    // events
    ActionButton::TActionHandler onAction = nullptr;

   protected:
    DigitalInput* _input = nullptr;
    ActionButton* _button = nullptr;
  };

} // namespace Victor::Components

#endif // ActionButtonInput_h
