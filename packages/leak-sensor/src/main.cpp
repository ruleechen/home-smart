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
extern "C" void onAccessoryIdentify(void (*callback)(const homekit_value_t value));

AppMain* appMain = nullptr;
String hostName;
String serialNumber;
bool connective = false;
bool isIdentified = false;

void identify(const homekit_value_t value) {
  isIdentified = !isIdentified;
  builtinLed.toggle();
}

DigitalSensor* sensor = nullptr;

// format: uint8; HAP section 9.50; 0 = Leak is not detected, 1 = Leak is detected
enum LeakDetected {
  LEAK_NOT_DETECTED = 0,
  LEAK_DETECTED     = 1,
};

LeakDetected boolToLeak(const bool value) {
  return value ? LEAK_DETECTED : LEAK_NOT_DETECTED;
}

String toLeakName(const uint8_t state) {
  return (
    state == LEAK_NOT_DETECTED ? F("Not Detected") :
    state == LEAK_DETECTED     ? F("Detected") : F("Unknown")
  );
}

void setLeakState(const LeakDetected leak, const bool notify) {
  leakState.value.uint8_value = leak;
  if (notify) {
    homekit_characteristic_notify(&leakState, leakState.value);
  }
  console.log()
    .section(F("leak"), toLeakName(leak))
    .section(F("notify"), GlobalHelpers::toYesNoName(notify));
}

void setLevelState(const int analog, const bool notify) {
  const auto percentage = map(analog, 0, 1023, 0, 100);
  levelState.value.int_value = percentage;
  if (notify) {
    homekit_characteristic_notify(&levelState, levelState.value);
  }
  console.log()
    .section(F("percentage"), String(percentage))
    .section(F("notify"), GlobalHelpers::toYesNoName(notify));
}

void setActiveState(const bool value, const bool notify) {
  activeState.value.bool_value = value;
  if (notify) {
    homekit_characteristic_notify(&activeState, activeState.value);
  }
  console.log()
    .section(F("active"), GlobalHelpers::toYesNoName(value))
    .section(F("notify"), GlobalHelpers::toYesNoName(notify));
}

void setup(void) {
  appMain = new AppMain();
  appMain->setup();

  // setup web
  appMain->webPortal->onServiceGet = [](std::vector<TextValueModel>& states, std::vector<TextValueModel>& buttons) {
    // states
    states.push_back({ .text = F("Service"),    .value = VICTOR_ACCESSORY_SERVICE_NAME });
    states.push_back({ .text = F("Identified"), .value = GlobalHelpers::toYesNoName(isIdentified) });
    states.push_back({ .text = F("Leak"),       .value = toLeakName(leakState.value.uint8_value) });
    states.push_back({ .text = F("Level"),      .value = GlobalHelpers::toPercentage(leakState.value.int_value) });
    states.push_back({ .text = F("Active"),     .value = GlobalHelpers::toYesNoName(activeState.value.bool_value) });
    states.push_back({ .text = F("Paired"),     .value = GlobalHelpers::toYesNoName(homekit_is_paired()) });
    states.push_back({ .text = F("Clients"),    .value = String(arduino_homekit_connected_clients_count()) });
    // buttons
    buttons.push_back({ .text = F("UnPair"), .value = F("UnPair") });
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
  hostName     = victorWiFi.getHostName();
  serialNumber = String(accessorySerialNumber.value.string_value) + "/" + victorWiFi.getHostId();
  accessoryNameInfo.value.string_value     = const_cast<char*>(hostName.c_str());
  accessorySerialNumber.value.string_value = const_cast<char*>(serialNumber.c_str());
  arduino_homekit_setup(&serverConfig);
  onAccessoryIdentify(identify);

  // connect leak sensor
  sensor = new DigitalSensor("/leak.json");
  sensor->onStateChange = [](const bool state) {
    builtinLed.flash();
    setLeakState(boolToLeak(state), connective);
  };
  setLeakState(boolToLeak(sensor->readState()), connective);

  // done
  console.log()
    .bracket(F("setup"))
    .section(F("complete"));
}

void loop(void) {
  arduino_homekit_loop();
  const auto isPaired = arduino_homekit_get_running_server()->paired;
  connective = victorWiFi.isConnective() && isPaired;
  const auto isLightSleep = victorWiFi.isLightSleepMode() && isPaired;
  appMain->loop(isLightSleep);
  sensor->loop();
}
