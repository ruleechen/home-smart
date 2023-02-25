#ifndef ActionButtonInterrupt_h
#define ActionButtonInterrupt_h

#include <vector>
#include "Console.h"
#include "Button/ActionButtonInput.h"

// should be a plural
// double click has maximum 4 times change
#ifndef VICTOR_DIGITAL_INPUT_MAX_CHANGES
#define VICTOR_DIGITAL_INPUT_MAX_CHANGES 4
#endif

namespace Victor::Components {

  struct InterruptContext {
    bool inputValue;
    unsigned long timestamp;
  };

  class ActionButtonInterrupt : public ActionButtonInput {
   public:
    ActionButtonInterrupt(uint8_t inputPin, uint8_t inputTrueValue);
    ~ActionButtonInterrupt();
    void loop() override;

   private:
   	static DigitalInput* _inputRef;
    volatile static bool _lastInputValue;
    static std::vector<InterruptContext*> _contexts;
    static void IRAM_ATTR _interruptHandler();
  };

} // namespace Victor::Components

#endif // ActionButtonInterrupt_h
