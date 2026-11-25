#ifndef DigitalSensor_h
#define DigitalSensor_h

#include "HAL/HalInterrupt.h"
#include "Pin/DigitalInput.h"
#include "StateSensor.h"

namespace Victor::Components {
  class DigitalSensor : public StateSensor<bool> {
   public:
    DigitalSensor(const char* pinJson);
    ~DigitalSensor();
    void loop();
    bool isAvailable();
    bool readState() override;

   private:
    DigitalInput* _input = nullptr;
    void _interruptHandler(const Victor::HAL::InterruptInfo info);
  };

} // namespace Victor::Components

#endif // DigitalSensor_h
