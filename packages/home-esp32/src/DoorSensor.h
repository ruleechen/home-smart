#ifndef DoorSensor_h
#define DoorSensor_h

#include <Arduino.h>
#include <HomeSpan.h>

namespace Victor::Components {

  struct MyDoor : Service::Door {
   public:
    SpanCharacteristic* currentPosition;
    SpanCharacteristic* targetPosition;
    SpanCharacteristic* positionState;

    MyDoor() : Service::Door() {
      /**
       * Percentage
       * Integer value from 0 to 100
       */
      currentPosition = new Characteristic::CurrentPosition(0);
      /**
       * Percentage
       * Integer value from 0 to 100
       */
      targetPosition = new Characteristic::TargetPosition(0);
      /**
       * 0 Decreasing - ”Going to the minimum value specified in metadata”
       * 1 Increasing - ”Going to the maximum value specified in metadata”
       * 2 Stopped
       */
      positionState = new Characteristic::PositionState(2);
    }
  };

  struct MyBattery : Service::BatteryService {
   public:
    SpanCharacteristic* batteryLevel;
    SpanCharacteristic* chargingState;
    SpanCharacteristic* statusLowBattery;

    MyBattery() : Service::BatteryService() {
      /**
       * Percentage
       * Integer value from 0 to 100
       */
      batteryLevel = new Characteristic::BatteryLevel(100);
      /**
       * 0 ”Not Charging”
       * 1 ”Charging”
       * 2 ”Not Chargeable”
       */
      chargingState = new Characteristic::ChargingState(0);
      /**
       * 0 ”Battery level is normal”
       * 1 ”Battery level is low”
       */
      statusLowBattery = new Characteristic::StatusLowBattery(0);
    }
  };

  class DoorSensor {
   public:
    DoorSensor();
    void setup();
    void loop();
    void setOpen();
    void setClosed();

   private:
    MyBattery* _batteryService = nullptr;
    MyDoor* _doorService = nullptr;
  };
} // namespace Victor::Components

#endif // DoorSensor_h
