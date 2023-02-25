#ifndef BinaryIO_h
#define BinaryIO_h

#include "DigitalInput.h"
#include "DigitalOutput.h"
#include "BinaryStorage.h"
#include "Button/ActionButtonInput.h"

namespace Victor::Components {
  class BinaryIO {
   public:
    BinaryIO(const char* settingFile = "/binary.json");
    ~BinaryIO();
    void loop();
    bool getOutputState();
    void setOutputState(bool value);
    // events
    ActionButton::TActionHandler onInputAction = nullptr;

   private:
    BinaryStorage* _storage = nullptr;
    ActionButtonInput* _input = nullptr;
    DigitalOutput* _output = nullptr;
    DigitalOutput* _output2 = nullptr;
  };

} // namespace Victor::Components

#endif // BinaryIO_h
