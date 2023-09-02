#include <Arduino.h>
#include <arduino_homekit_server.h>

#include <AppMain/AppMain.h>
#include <GlobalHelpers.h>
#include <Timer/TimesCounter.h>
#include <Button/ActionButtonInterrupt.h>

using namespace Victor;
using namespace Victor::Components;

extern "C" homekit_characteristic_t eventState1;
extern "C" homekit_characteristic_t eventState2;
extern "C" homekit_characteristic_t eventState3;
extern "C" homekit_characteristic_t accessoryNameInfo;
extern "C" homekit_characteristic_t accessorySerialNumber;
extern "C" homekit_server_config_t serverConfig;
extern "C" void onAccessoryIdentify(void (*callback)(const homekit_value_t value));

AppMain* appMain = nullptr;
bool connective = false;

TimesCounter times(1000);
ActionButtonInterrupt* button1 = nullptr;
ActionButtonInterrupt* button2 = nullptr;
ActionButtonInterrupt* button3 = nullptr;

String hostName;
String serialNumber;

// format: uint8; HAP section 9.75; 0 = Single Press, 1 = Double Press, 2 = Long Press
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

void setEventState(const uint8_t index, const ProgrammableSwitchEvent value, const bool notify) {
  builtinLed.flash();
  homekit_characteristic_t eventState;
  if (index == 1) {
    eventState = eventState1;
  } else if (index == 2) {
    eventState = eventState2;
  } else if (index == 3) {
    eventState = eventState3;
  } else {
    return;
  }
  eventState.value.uint8_value = value;
  if (notify) {
    homekit_characteristic_notify(&eventState, eventState.value);
  }
  console.log()
    .section(F("button"), String(index))
    .section(F("state"), toProgrammableSwitchEventName(value))
    .section(F("notify"), GlobalHelpers::toYesNoName(notify));
}

void buttonAction(const uint8_t index, const ButtonAction action) {
  console.log()
    .section(F("button"), String(index))
    .section(F("action"), String(action));
  if (action == BUTTON_ACTION_PRESSED) {
    setEventState(index, BUTTON_EVENT_SINGLE_PRESS, connective);
  } else if (action == BUTTON_ACTION_RELEASED) {
    times.count(); // count only for real button released
  } else if (action == BUTTON_ACTION_DOUBLE_PRESSED) {
    setEventState(index, BUTTON_EVENT_DOUBLE_PRESS, connective);
    builtinLed.flash(500);
  } else if (action == BUTTON_ACTION_PRESSED_HOLD_L1) {
    setEventState(index, BUTTON_EVENT_LONG_PRESS, connective);
    builtinLed.flash(1000);
  } else if (action == BUTTON_ACTION_PRESSED_HOLD_L2) {
    homekit_server_reset();
    ESP.restart();
  }
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
    states.push_back({ .text = F("State1"),  .value = toProgrammableSwitchEventName(eventState1.value.uint8_value) });
    states.push_back({ .text = F("State2"),  .value = toProgrammableSwitchEventName(eventState2.value.uint8_value) });
    states.push_back({ .text = F("State3"),  .value = toProgrammableSwitchEventName(eventState3.value.uint8_value) });
    states.push_back({ .text = F("Paired"),  .value = GlobalHelpers::toYesNoName(homekit_is_paired()) });
    states.push_back({ .text = F("Clients"), .value = String(arduino_homekit_connected_clients_count()) });
    // buttons
    buttons.push_back({ .text = F("UnPair"), .value = F("UnPair") });
    // event button1
    buttons.push_back({ .text = F("Single Press 1"), .value = F("SinglePress1") });
    buttons.push_back({ .text = F("Double Press 1"), .value = F("DoublePress1") });
    buttons.push_back({ .text = F("Long Press 1"),   .value = F("LongPress1")   });
    // event button2
    buttons.push_back({ .text = F("Single Press 2"), .value = F("SinglePress2") });
    buttons.push_back({ .text = F("Double Press 2"), .value = F("DoublePress2") });
    buttons.push_back({ .text = F("Long Press 2"),   .value = F("LongPress2")   });
    // event button3
    buttons.push_back({ .text = F("Single Press 3"), .value = F("SinglePress3") });
    buttons.push_back({ .text = F("Double Press 3"), .value = F("DoublePress3") });
    buttons.push_back({ .text = F("Long Press 3"),   .value = F("LongPress3")   });
  };
  appMain->webPortal->onServicePost = [](const String& value) {
    if (value == F("UnPair")) {
      homekit_server_reset();
      ESP.restart();
    } else if (value == F("SinglePress1")) {
      setEventState(1, BUTTON_EVENT_SINGLE_PRESS, connective);
    } else if (value == F("DoublePress1")) {
      setEventState(1, BUTTON_EVENT_DOUBLE_PRESS, connective);
    } else if (value == F("LongPress1")) {
      setEventState(1, BUTTON_EVENT_LONG_PRESS, connective);
    } else if (value == F("SinglePress2")) {
      setEventState(2, BUTTON_EVENT_SINGLE_PRESS, connective);
    } else if (value == F("DoublePress2")) {
      setEventState(2, BUTTON_EVENT_DOUBLE_PRESS, connective);
    } else if (value == F("LongPress2")) {
      setEventState(2, BUTTON_EVENT_LONG_PRESS, connective);
    } else if (value == F("SinglePress3")) {
      setEventState(3, BUTTON_EVENT_SINGLE_PRESS, connective);
    } else if (value == F("DoublePress3")) {
      setEventState(3, BUTTON_EVENT_DOUBLE_PRESS, connective);
    } else if (value == F("LongPress3")) {
      setEventState(3, BUTTON_EVENT_LONG_PRESS, connective);
    }
  };

  // setup homekit server
  hostName     = victorWifi.getHostName();
  serialNumber = String(accessorySerialNumber.value.string_value) + "/" + victorWifi.getHostId();
  accessoryNameInfo.value.string_value     = const_cast<char*>(hostName.c_str());
  accessorySerialNumber.value.string_value = const_cast<char*>(serialNumber.c_str());
  eventState1.getter = []() { return HOMEKIT_NULL_CPP(); }; // See HAP section 9.75; Should always return "null" for reading
  eventState2.getter = []() { return HOMEKIT_NULL_CPP(); };
  eventState3.getter = []() { return HOMEKIT_NULL_CPP(); };
  // eventState1.setter = [](const homekit_value_t value) { setEventState(1, ProgrammableSwitchEvent(value.uint8_value), connective); };
  arduino_homekit_setup(&serverConfig);
  onAccessoryIdentify([](const homekit_value_t value) { builtinLed.toggle(); });

  // buttons
  const auto buttonJson1 = new PinStorage("/button1.json");
  const auto buttonJson2 = new PinStorage("/button2.json");
  const auto buttonJson3 = new PinStorage("/button3.json");
  const auto buttonPin1 = buttonJson1->load();
  const auto buttonPin2 = buttonJson2->load();
  const auto buttonPin3 = buttonJson3->load();
  if (buttonPin1 != nullptr && buttonPin1->enable) {
    button1 = new ActionButtonInterrupt(buttonPin1);
    button1->onAction = [](const ButtonAction action) {
      buttonAction(1, action);
    };
  }
  if (buttonPin2 != nullptr && buttonPin2->enable) {
    button2 = new ActionButtonInterrupt(buttonPin2);
    button2->onAction = [](const ButtonAction action) {
      buttonAction(2, action);
    };
  }
  if (buttonPin3 != nullptr && buttonPin3->enable) {
    button3 = new ActionButtonInterrupt(buttonPin3);
    button3->onAction = [](const ButtonAction action) {
      buttonAction(3, action);
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
  if (button1 != nullptr) { button1->loop(); }
  if (button2 != nullptr) { button2->loop(); }
  if (button3 != nullptr) { button3->loop(); }
}
