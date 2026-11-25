#ifndef BinaryIO_h
#define BinaryIO_h

#include "Pin/DigitalOutput.h"
#include "Button/ActionButtonInput.h"
#include "Storage/DigitalStateStorage/DigitalStateStorage.h"

namespace Victor::Components {
  struct BinaryIOProps {
    const char* buttonJson  = nullptr; // = "/button.json";
    const char* outputJson  = nullptr; // = "/output.json";
    const char* outputJson2 = nullptr; // = "/output2.json";
    const char* stateJson   = nullptr; // = "/state.json";
  };

  class BinaryIO {
   public:
    BinaryIO(const BinaryIOProps* props);
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
