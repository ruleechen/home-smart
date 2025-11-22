#ifndef ActionButtonInput_h
#define ActionButtonInput_h

#include "Pin/DigitalInput.h"
#include "Button/ActionButton.h"

namespace Victor::Components {

  class ActionButtonInput {
   public:
    ActionButtonInput(PinConfig* config);
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
