#ifndef BinaryIO_h
#define BinaryIO_h

#include "Storage/PinStorage/PinStorage.h"
#include "Storage/DigitalStateStorage/DigitalStateStorage.h"
#include "Pin/DigitalInput.h"
#include "Pin/DigitalOutput.h"
#include "Button/ActionButtonInput.h"

namespace Victor::Components {
  class BinaryIO {
   public:
    BinaryIO();
    ~BinaryIO();
    void loop();
    bool getOutputState();
    void setOutputState(bool value);
    // events
    ActionButton::TActionHandler onButtonAction = nullptr;

   private:
    ActionButtonInput* _button = nullptr;
    DigitalOutput* _output = nullptr;
    DigitalOutput* _output2 = nullptr;
    DigitalStateStorage* _stateStorage = nullptr;
  };

} // namespace Victor::Components

#endif // BinaryIO_h
