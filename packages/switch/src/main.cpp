#include <Arduino.h>
#include <arduino_homekit_server.h>

#include <AppMain/AppMain.h>
#include <GlobalHelpers.h>
#include <Timer/TimesCounter.h>
#include <BinaryIO/BinaryIO.h>

using namespace Victor;
using namespace Victor::Components;

extern "C" homekit_characteristic_t onState;
extern "C" homekit_characteristic_t accessoryNameInfo;
extern "C" homekit_characteristic_t accessorySerialNumber;
extern "C" homekit_server_config_t serverConfig;
extern "C" void onAccessoryIdentify(void (*callback)(const homekit_value_t value));

AppMain* appMain = nullptr;
bool connective = false;

TimesCounter times(1000);
BinaryIO* binaryIO = nullptr;

String hostName;
String serialNumber;

void setOnState(const bool value, const bool notify) {
  ESP.wdtFeed();
  builtinLed.flash();
  onState.value.bool_value = value;
  if (notify) {
    homekit_characteristic_notify(&onState, onState.value);
  }
  binaryIO->setOutputState(value);
  console.log()
    .section(F("state"), GlobalHelpers::toOnOffName(value))
    .section(F("notify"), GlobalHelpers::toYesNoName(notify));
}

void setup(void) {
  appMain = new AppMain();
  appMain->setup();

  // counter
  times.onCount = [](uint8_t count) {
    if (count == 20) {
      homekit_server_reset();
      ESP.restart();
    }
  };

  // setup web
  appMain->webPortal->onServiceGet = [](std::vector<TextValueModel>& states, std::vector<TextValueModel>& buttons) {
    // states
    states.push_back({ .text = F("Service"), .value = VICTOR_ACCESSORY_SERVICE_NAME });
    states.push_back({ .text = F("State"),   .value = GlobalHelpers::toOnOffName(onState.value.bool_value) });
    states.push_back({ .text = F("Paired"),  .value = GlobalHelpers::toYesNoName(homekit_is_paired()) });
    states.push_back({ .text = F("Clients"), .value = String(arduino_homekit_connected_clients_count()) });
    // buttons
    buttons.push_back({ .text = F("UnPair"), .value = F("UnPair") });
    buttons.push_back({ .text = F("Toggle"), .value = F("Toggle") });
  };
  appMain->webPortal->onServicePost = [](const String& value) {
    if (value == F("UnPair")) {
      homekit_server_reset();
      ESP.restart();
    } else if (value == F("Toggle")) {
      setOnState(!onState.value.bool_value, connective);
    }
  };

  // setup homekit server
  hostName     = victorWifi.getHostName();
  serialNumber = String(accessorySerialNumber.value.string_value) + "/" + victorWifi.getHostId();
  accessoryNameInfo.value.string_value     = const_cast<char*>(hostName.c_str());
  accessorySerialNumber.value.string_value = const_cast<char*>(serialNumber.c_str());
  onState.setter = [](const homekit_value_t value) { setOnState(value.bool_value, connective); };
  arduino_homekit_setup(&serverConfig);
  onAccessoryIdentify([](const homekit_value_t value) { builtinLed.toggle(); });

  // setup binary io
  binaryIO = new BinaryIO();
  setOnState(binaryIO->getOutputState(), false);
  binaryIO->onButtonAction = [](const ButtonAction action) {
    console.log()
      .bracket(F("button"))
      .section(F("action"), String(action));
    if (action == BUTTON_ACTION_PRESSED) {
      const auto outputValue = binaryIO->getOutputState();
      setOnState(!outputValue, connective); // toggle
    } else if (action == BUTTON_ACTION_RELEASED) {
      times.count(); // count only for real button released
    } else if (action == BUTTON_ACTION_DOUBLE_PRESSED) {
      builtinLed.flash(500);
      const auto enable = victorWifi.isLightSleepMode();
      victorWifi.enableAP(enable); // toggle enabling ap
    } else if (action == BUTTON_ACTION_PRESSED_HOLD_L1) {
      ESP.restart();
    } else if (action == BUTTON_ACTION_PRESSED_HOLD_L2) {
      homekit_server_reset();
      ESP.restart();
    }
  };

  // done
  console.log()
    .bracket(F("setup"))
    .section(F("complete"));
}

void loop(void) {
  arduino_homekit_loop();
  const auto isPaired = arduino_homekit_get_running_server()->paired;
  connective = victorWifi.isConnective() && isPaired;
  const auto isLightSleep = victorWifi.isLightSleepMode() && isPaired;
  appMain->loop(isLightSleep);
  binaryIO->loop();
}
