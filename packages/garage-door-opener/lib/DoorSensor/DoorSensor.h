#ifndef DoorSensor_h
#define DoorSensor_h

#include <Arduino.h>
#include <FunctionalInterrupt.h>
#include "Storage/PinStorage/PinStorage.h"
#include "Pin/DigitalInput.h"
#include "Sensor/StateSensor.h"
#include "DoorStorage.h"

namespace Victor::Components {
  class DoorSensor : public StateSensor<CurrentDoorState> {
   public:
    DoorSensor();
    ~DoorSensor();
    CurrentDoorState readState() override;

   private:
    DigitalInput* _openSensor = nullptr;
    DigitalInput* _closedSensor = nullptr;
    void _interruptHandler(const InterruptInfo info);
  };

} // namespace Victor::Components

#endif // DoorSensor_h
