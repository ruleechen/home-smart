#include <Arduino.h>
#include <arduino_homekit_server.h>

#include <AppMain/AppMain.h>
#include <GlobalHelpers.h>
#include <Sensor/DigitalSensor.h>

using namespace Victor;
using namespace Victor::Components;

extern "C" homekit_characteristic_t leakState;
extern "C" homekit_characteristic_t levelState;
extern "C" homekit_characteristic_t activeState;
extern "C" homekit_characteristic_t accessoryNameInfo;
extern "C" homekit_characteristic_t accessorySerialNumber;
extern "C" homekit_server_config_t serverConfig;

AppMain* appMain = nullptr;
bool connective = false;

DigitalSensor* sensor = nullptr;

String hostName;
String serialNumber;

enum Leak {
  LEAK_NOT_DETECTED = 0,
  LEAK_DETECTED     = 1,
};

String toLeakName(const uint8_t state) {
  return (
    state == LEAK_NOT_DETECTED ? F("Not Detected") :
    state == LEAK_DETECTED     ? F("Detected") : F("Unknown")
  );
}

void setLeakState(const bool value, const bool notify) {
  const auto leak = value ? LEAK_DETECTED : LEAK_NOT_DETECTED;
  leakState.value.uint8_value = leak;
  if (notify) {
    homekit_characteristic_notify(&leakState, leakState.value);
  }
  console.log().section(F("leak"), toLeakName(leak));
}

void setLevelState(const int analog, const bool notify) {
  const auto percentage = map(analog, 0, 1023, 0, 100);
  levelState.value.int_value = percentage;
  if (notify) {
    homekit_characteristic_notify(&levelState, levelState.value);
  }
}

void setActiveState(const bool value, const bool notify) {
  activeState.value.bool_value = value;
  if (notify) {
    homekit_characteristic_notify(&activeState, activeState.value);
  }
}

void setup(void) {
  appMain = new AppMain();
  appMain->setup();

  // setup web
  appMain->webPortal->onServiceGet = [](std::vector<TextValueModel>& states, std::vector<TextValueModel>& buttons) {
    // states
    states.push_back({ .text = F("Service"), .value = VICTOR_ACCESSORY_SERVICE_NAME });
    states.push_back({ .text = F("Leak"),    .value = toLeakName(leakState.value.uint8_value) });
    states.push_back({ .text = F("Level"),   .value = String(leakState.value.int_value) + F("%")  });
    states.push_back({ .text = F("Active"),  .value = GlobalHelpers::toYesNoName(activeState.value.bool_value) });
    states.push_back({ .text = F("Paired"),  .value = GlobalHelpers::toYesNoName(homekit_is_paired()) });
    states.push_back({ .text = F("Clients"), .value = String(arduino_homekit_connected_clients_count()) });
    // buttons
    buttons.push_back({ .text = F("UnPair"),   .value = F("UnPair") });
  };
  appMain->webPortal->onServicePost = [](const String& value) {
    if (value == F("UnPair")) {
      homekit_server_reset();
      ESP.restart();
    }
  };
  appMain->onHeartbeat = []() {
    builtinLed.flash();
    const auto level = analogRead(A0);
    setLevelState(level, connective);
    setActiveState(true, connective);
  };

  // setup homekit server
  hostName     = victorWifi.getHostName();
  serialNumber = String(accessorySerialNumber.value.string_value) + "/" + victorWifi.getHostId();
  accessoryNameInfo.value.string_value     = const_cast<char*>(hostName.c_str());
  accessorySerialNumber.value.string_value = const_cast<char*>(serialNumber.c_str());
  arduino_homekit_setup(&serverConfig);

  // connect leak sensor
  sensor = new DigitalSensor("/leak.json");
  sensor->onStateChange = [](const bool state) {
    builtinLed.flash();
    setLeakState(state, connective);
  };
  setLeakState(sensor->readState(), connective);

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
  sensor->loop();
}
