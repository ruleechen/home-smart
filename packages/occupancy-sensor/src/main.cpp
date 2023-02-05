#include <Arduino.h>
#include <arduino_homekit_server.h>

#include <AppMain/AppMain.h>
#include <GlobalHelpers.h>

#include "OccupancyStorage.h"
#include "OccupancySensor.h"

using namespace Victor;
using namespace Victor::Components;

extern "C" homekit_characteristic_t occupancyState;
extern "C" homekit_characteristic_t activeState;
extern "C" homekit_characteristic_t accessoryNameInfo;
extern "C" homekit_characteristic_t accessorySerialNumber;
extern "C" homekit_server_config_t serverConfig;

AppMain* appMain = nullptr;
bool connective = false;

OccupancySensor* sensor = nullptr;

String hostName;
String serialNumber;

enum Occupancy {
  OCCUPANCY_NOT_DETECTED = 0,
  OCCUPANCY_DETECTED     = 1,
};

String toOccupancyName(const uint8_t state) {
  return (
    state == OCCUPANCY_NOT_DETECTED ? F("Not Detected") :
    state == OCCUPANCY_DETECTED     ? F("Detected") : F("Unknown")
  );
}

void setOccupancyState(const bool value, const bool notify) {
  ESP.wdtFeed();
  builtinLed.flash();
  const auto occupancy = value ? OCCUPANCY_DETECTED : OCCUPANCY_NOT_DETECTED;
  occupancyState.value.uint8_value = occupancy;
  if (notify) {
    homekit_characteristic_notify(&occupancyState, occupancyState.value);
  }
  console.log().section(F("occupancy"), toOccupancyName(occupancy));
}

void setActiveState(const bool value, const bool notify) {
  builtinLed.flash();
  activeState.value.bool_value = value;
  if (notify) {
    homekit_characteristic_notify(&activeState, activeState.value);
  }
  console.log().section(F("active"), GlobalHelpers::toYesNoName(value));
}

void setup(void) {
  appMain = new AppMain();
  appMain->setup();

  // setup web
  appMain->webPortal->onServiceGet = [](std::vector<TextValueModel>& states, std::vector<TextValueModel>& buttons) {
    // states
    states.push_back({ .text = F("Service"),   .value = VICTOR_ACCESSORY_SERVICE_NAME });
    states.push_back({ .text = F("Occupancy"), .value = toOccupancyName(occupancyState.value.uint8_value) });
    states.push_back({ .text = F("Active"),    .value = GlobalHelpers::toYesNoName(activeState.value.bool_value) });
    states.push_back({ .text = F("Paired"),    .value = GlobalHelpers::toYesNoName(homekit_is_paired()) });
    states.push_back({ .text = F("Clients"),   .value = String(arduino_homekit_connected_clients_count()) });
    // buttons
    buttons.push_back({ .text = F("UnPair"),   .value = F("UnPair") });
  };
  appMain->webPortal->onServicePost = [](const String& value) {
    if (value == F("UnPair")) {
      homekit_server_reset();
      ESP.restart();
    }
  };

  // setup homekit server
  hostName     = victorWifi.getHostName();
  serialNumber = String(accessorySerialNumber.value.string_value) + "/" + victorWifi.getHostId();
  accessoryNameInfo.value.string_value     = const_cast<char*>(hostName.c_str());
  accessorySerialNumber.value.string_value = const_cast<char*>(serialNumber.c_str());
  arduino_homekit_setup(&serverConfig);

  // connect occupancy sensor
  const auto occupancySetting = occupancyStorage.load();
  if (occupancySetting->sensorPin > -1) {
    sensor = new OccupancySensor();
    sensor->onStateChange = [](const bool state) {
      setOccupancyState(state, connective);
      setActiveState(true, connective);
    };
    setOccupancyState(sensor->readState(), connective);
  }

  // done
  console.log()
    .bracket(F("setup"))
    .section(F("complete"));
}

void loop(void) {
  arduino_homekit_loop();
  const auto isPaired = arduino_homekit_get_running_server()->paired;
  connective = victorWifi.isLightSleepMode() && isPaired;
  appMain->loop(connective);
  // sensor
  if (sensor != nullptr) {
    sensor->loop();
  }
}
