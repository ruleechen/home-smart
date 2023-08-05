#include <Arduino.h>
#include <arduino_homekit_server.h>

#include <AppMain/AppMain.h>
#include <GlobalHelpers.h>
#include <Timer/TimesCounter.h>
#include <Button/ActionButtonInterrupt.h>

using namespace Victor;
using namespace Victor::Components;

extern "C" homekit_characteristic_t eventState;
extern "C" homekit_characteristic_t accessoryNameInfo;
extern "C" homekit_characteristic_t accessorySerialNumber;
extern "C" homekit_server_config_t serverConfig;
extern "C" void onAccessoryIdentify(void (*callback)(const homekit_value_t value));

AppMain* appMain = nullptr;
bool connective = false;

TimesCounter times(1000);
ActionButtonInterrupt* button = nullptr;

String hostName;
String serialNumber;

enum ProgrammableSwitchEvent {
  BUTTON_EVENT_SINGLE_PRESS = 0,
  BUTTON_EVENT_DOUBLE_PRESS = 1,
  BUTTON_EVENT_LONG_PRESS   = 2,
};

String toProgrammableSwitchEventName(const uint8_t state) {
  return (
    state == BUTTON_EVENT_SINGLE_PRESS ? F("Single Press") :
    state == BUTTON_EVENT_DOUBLE_PRESS ? F("Double Press") :
    state == BUTTON_EVENT_LONG_PRESS   ? F("Long Press") : F("Unknown")
  );
}

void setEventState(const ProgrammableSwitchEvent value, const bool notify) {
  ESP.wdtFeed();
  builtinLed.flash();
  eventState.value.uint8_value = value;
  if (notify) {
    homekit_characteristic_notify(&eventState, eventState.value);
  }
  console.log().section(F("state"), toProgrammableSwitchEventName(value));
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
    states.push_back({ .text = F("State"),   .value = toProgrammableSwitchEventName(eventState.value.uint8_value) });
    states.push_back({ .text = F("Paired"),  .value = GlobalHelpers::toYesNoName(homekit_is_paired()) });
    states.push_back({ .text = F("Clients"), .value = String(arduino_homekit_connected_clients_count()) });
    // buttons
    buttons.push_back({ .text = F("UnPair"), .value = F("UnPair") });
    // event buttons
    buttons.push_back({ .text = F("Single Press"), .value = F("SinglePress") });
    buttons.push_back({ .text = F("Double Press"), .value = F("DoublePress") });
    buttons.push_back({ .text = F("Long Press"),   .value = F("LongPress") });
  };
  appMain->webPortal->onServicePost = [](const String& value) {
    if (value == F("UnPair")) {
      homekit_server_reset();
      ESP.restart();
    } else if (value == F("SinglePress")) {
      setEventState(BUTTON_EVENT_SINGLE_PRESS, connective);
    } else if (value == F("DoublePress")) {
      setEventState(BUTTON_EVENT_DOUBLE_PRESS, connective);
    } else if (value == F("LongPress")) {
      setEventState(BUTTON_EVENT_LONG_PRESS, connective);
    }
  };

  // setup homekit server
  hostName     = victorWifi.getHostName();
  serialNumber = String(accessorySerialNumber.value.string_value) + "/" + victorWifi.getHostId();
  accessoryNameInfo.value.string_value     = const_cast<char*>(hostName.c_str());
  accessorySerialNumber.value.string_value = const_cast<char*>(serialNumber.c_str());
  eventState.getter = []() { return HOMEKIT_NULL_CPP(); };
  arduino_homekit_setup(&serverConfig);
  onAccessoryIdentify([](const homekit_value_t value) { builtinLed.toggle(); });

  // button
  const auto buttonJson = new PinStorage("/button.json");
  const auto buttonPin = buttonJson->load();
  if (buttonPin->enable) {
    button = new ActionButtonInterrupt(buttonPin);
    button->onAction = [](const ButtonAction action) {
      console.log()
        .bracket(F("button"))
        .section(F("action"), String(action));
      if (action == BUTTON_ACTION_PRESSED) {
        builtinLed.flash();
        setEventState(BUTTON_EVENT_SINGLE_PRESS, connective);
      } else if (action == BUTTON_ACTION_DOUBLE_PRESSED) {
        builtinLed.flash(500);
        setEventState(BUTTON_EVENT_DOUBLE_PRESS, connective);
        const auto enable = victorWifi.isLightSleepMode();
        victorWifi.enableAP(enable); // toggle enabling ap
      } else if (action == BUTTON_ACTION_PRESSED_HOLD_L1) {
        setEventState(BUTTON_EVENT_LONG_PRESS, connective);
        builtinLed.flash(1000);
      } else if (action == BUTTON_ACTION_PRESSED_HOLD_L2) {
        homekit_server_reset();
        ESP.restart();
      }
    };
  }

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
  if (button != nullptr) {
    button->loop();
  }
}
