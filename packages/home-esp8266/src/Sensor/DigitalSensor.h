#ifndef DigitalSensor_h
#define DigitalSensor_h

#include <Arduino.h>
#include <Pin/PinStorage.h>
#include <Pin/DigitalInput.h>
#include <Sensor/StateSensor.h>

namespace Victor::Components {
  class DigitalSensor : public StateSensor<bool> {
   public:
    DigitalSensor(const char* pinJson);
    ~DigitalSensor();
    bool readState() override;

   private:
    DigitalInput* _input = nullptr;
    static void IRAM_ATTR _interruptHandler();
  };

} // namespace Victor::Components

#endif // DigitalSensor_h
