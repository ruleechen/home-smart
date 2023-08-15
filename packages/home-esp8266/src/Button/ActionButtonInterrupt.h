#ifndef ActionButtonInterrupt_h
#define ActionButtonInterrupt_h

#include <vector>
#include <FunctionalInterrupt.h>
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
    ActionButtonInterrupt(PinConfig* config);
    ~ActionButtonInterrupt();
    void loop() override;

   private:
    bool _lastInputValue;
    std::vector<InterruptContext*> _contexts = {};
    void _interruptHandler(const InterruptInfo info);
  };

} // namespace Victor::Components

#endif // ActionButtonInterrupt_h
