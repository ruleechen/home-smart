#ifndef DigitalSensor_h
#define DigitalSensor_h

#include <Arduino.h>
#include <FunctionalInterrupt.h>
#include "Console.h"
#include "Pin/PinStorage.h"
#include "Pin/DigitalInput.h"
#include "Sensor/StateSensor.h"

namespace Victor::Components {
  class DigitalSensor : public StateSensor<bool> {
   public:
    DigitalSensor(const char* pinJson);
    ~DigitalSensor();
    bool isAvailable();
    bool readState() override;

   private:
    DigitalInput* _input = nullptr;
    void _interruptHandler(const InterruptInfo info);
  };

} // namespace Victor::Components

#endif // DigitalSensor_h
