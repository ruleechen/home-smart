#include "DoorSensor.h"

namespace Victor::Components {

  const char* ssid = "HUAWEI-1A11JT";
  const char* password = "1616181818";

  DoorSensor::DoorSensor() { }

  void DoorSensor::setup() {
    #ifdef LED_BUILTIN
      homeSpan.setStatusPin(LED_BUILTIN);
    #endif
    homeSpan.setWifiCredentials(ssid, password);
    homeSpan.setPairingCode("15980935");
    homeSpan.begin(Category::Sensors, "Victor-Door-Sensor");

    new SpanAccessory();
      new Service::AccessoryInformation();
      new Characteristic::Identify();
      new Characteristic::Name("Victor-Door");
      new Characteristic::Manufacturer("RuleeSmart");
      new Characteristic::Model("Victor-Door-ESP32");
      new Characteristic::SerialNumber("202206260023");
      new Characteristic::HardwareRevision("22.4.30");
      new Characteristic::FirmwareRevision("22.4.30");
      _batteryService = new MyBattery();
      _doorService = new MyDoor();
  }

  void DoorSensor::loop() {
    homeSpan.poll();
  }

  void DoorSensor::setOpen() {
    _doorService->targetPosition->setVal(100);
  }

  void DoorSensor::setClosed() {
    _doorService->targetPosition->setVal(0);
  }

} // namespace Victor::Components
