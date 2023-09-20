#include <Arduino.h>
#include <arduino_homekit_server.h>

#include <AppMain/AppMain.h>
#include <GlobalHelpers.h>
#include <Pin/DigitalOutput.h>
#include <Sensor/DigitalSensor.h>
#include <Button/ActionButtonInterrupt.h>
#include <Storage/DigitalStateStorage/DigitalStateStorage.h>

using namespace Victor;
using namespace Victor::Components;

extern "C" homekit_characteristic_t activeState;
extern "C" homekit_characteristic_t inUseState;
extern "C" homekit_characteristic_t valveTypeState;
extern "C" homekit_characteristic_t accessoryNameInfo;
extern "C" homekit_characteristic_t accessorySerialNumber;
extern "C" homekit_server_config_t serverConfig;
extern "C" void onAccessoryIdentify(void (*callback)(const homekit_value_t value));

AppMain* appMain = nullptr;
String hostName;
String serialNumber;
bool connective = false;
bool isIdentified = false;
String lastActName;
unsigned long lastActMillis = 0;

void identify(const homekit_value_t value) {
  isIdentified = !isIdentified;
  builtinLed.toggle();
}

ActionButtonInterrupt* button = nullptr;
DigitalStateStorage* storage = nullptr;
DigitalSensor* inUseSensor = nullptr;
DigitalSensor* notInUseSensor = nullptr;
DigitalOutput* motorLine1 = nullptr;
DigitalOutput* motorLine2 = nullptr;

void motorForward() {
  if (motorLine1 != nullptr) { motorLine1->setValue(true); }
  if (motorLine2 != nullptr) { motorLine2->setValue(false); }
}

void motorBackward() {
  if (motorLine1 != nullptr) { motorLine1->setValue(false); }
  if (motorLine2 != nullptr) { motorLine2->setValue(true); }
}

void motorIdle() {
  if (motorLine1 != nullptr) { motorLine1->setValue(false); }
  if (motorLine2 != nullptr) { motorLine2->setValue(false); }
}

void motorStop() {
  if (motorLine1 != nullptr) { motorLine1->setValue(true); }
  if (motorLine2 != nullptr) { motorLine2->setValue(true); }
}

// format: uint8; HAP section 9.3; 0 = Inactive, 1 = Active
enum Active {
  INACTIVE = 0,
  ACTIVE   = 1,
};

// format: uint8; HAP section 9.48; 0 = Not in use, 1 = In use
enum InUse {
  NOT_IN_USE = 0,
  IN_USE     = 1,
};

// format: uint8; HAP section 9.124; 0 = Generic valve, 1 = Irrigation, 2 = Shower head, 3 = Water faucet
enum ValveType {
  GENERIC_VALVE = 0,
  IRRIGATION    = 1,
  SHOWER_HEAD   = 2,
  WATER_FAUCET  = 3,
};

Active uint8ToActive(const uint8_t value) {
  return (value == 1) ? ACTIVE : INACTIVE;
}

Active boolToActive(const bool value) {
  return value ? ACTIVE : INACTIVE;
}

bool activeToBool(const Active value) {
  return (value == ACTIVE) ? true : false;
}

InUse boolToInUse(const bool value) {
  return value ? IN_USE : NOT_IN_USE;
}

String toActiveName(const Active active) {
  return (
    active == INACTIVE ? F("Inactive") :
    active == ACTIVE   ? F("Active") : F("Unknown")
  );
}

String toActiveName(const uint8_t value) {
  const auto active = uint8ToActive(value);
  return toActiveName(active);
}

String toInUseName(const uint8_t state) {
  return (
    state == NOT_IN_USE ? F("Not in use") :
    state == IN_USE     ? F("In use") : F("Unknown")
  );
}

String toValveTypeName(const uint8_t state) {
  return (
    state == GENERIC_VALVE ? F("Generic valve") :
    state == IRRIGATION    ? F("Irrigation") :
    state == SHOWER_HEAD   ? F("Shower head") :
    state == WATER_FAUCET  ? F("Water faucet") : F("Unknown")
  );
}

void setValveTypeState(const String actName, const ValveType value, const bool notify) {
  lastActName = actName;
  lastActMillis = millis();
  valveTypeState.value.uint8_value = value;
  if (notify) {
    homekit_characteristic_notify(&valveTypeState, valveTypeState.value);
  }
  console.log()
    .section(F("act"), actName)
    .section(F("state"), toValveTypeName(value))
    .section(F("notify"), GlobalHelpers::toYesNoName(notify));
}

void setInUseState(const String actName, const InUse value, const bool notify) {
  lastActName = actName;
  lastActMillis = millis();
  inUseState.value.uint8_value = value;
  if (notify) {
    homekit_characteristic_notify(&inUseState, inUseState.value);
  }
  console.log()
    .section(F("act"), actName)
    .section(F("state"), toInUseName(value))
    .section(F("notify"), GlobalHelpers::toYesNoName(notify));
}

void saveActiveState(const Active value) {
  auto state = storage->load();
  if (state != nullptr && state->save) {
    const auto boolValue = activeToBool(value);
    if (*state->currentValue != boolValue) {
      delete state->currentValue;
      state->currentValue = new bool(boolValue);
      storage->save(state);
    }
  }
}

void setActiveState(const String actName, const Active value, const bool notify) {
  lastActName = actName;
  lastActMillis = millis();
  activeState.value.uint8_value = value;
  if (notify) {
    homekit_characteristic_notify(&activeState, activeState.value);
  }
  saveActiveState(value);
  console.log()
    .section(F("act"), actName)
    .section(F("state"), toActiveName(value))
    .section(F("notify"), GlobalHelpers::toYesNoName(notify));
}

void setInUse(const String actName, const InUse inUse, const bool notify) {
  // set active state first
  const auto active = uint8ToActive(activeState.value.uint8_value);
  if (inUse == IN_USE && active == INACTIVE) {
    setActiveState(actName, ACTIVE, notify);
  } else if (inUse == NOT_IN_USE && active == ACTIVE) {
    setActiveState(actName, INACTIVE, notify);
  }
  // then set in-use state
  setInUseState(actName, inUse, notify);
}

void setActive(const String actName, const Active active, const bool notify) {
  // set active state first
  setActiveState(actName, active, notify);
  // then set in-use state
  if (active == ACTIVE) {
    motorBackward();
    if (inUseSensor == nullptr || !inUseSensor->isAvailable()) {
      setInUseState(actName, IN_USE, notify);
    }
  } else if (active == INACTIVE) {
    motorForward();
    if (notInUseSensor == nullptr || !notInUseSensor->isAvailable()) {
      setInUseState(actName, NOT_IN_USE, notify);
    }
  }
}

void toggleActive(const String actName, const bool notify) {
  const auto active = uint8ToActive(activeState.value.uint8_value);
  const auto newValue = (active == ACTIVE) ? INACTIVE : ACTIVE;
  setActive(actName, newValue, notify);
}

void setup(void) {
  appMain = new AppMain();
  appMain->setup();

  // setup web
  appMain->webPortal->onServiceGet = [](std::vector<TextValueModel>& states, std::vector<TextValueModel>& buttons) {
    // states
    states.push_back({ .text = F("Service"),    .value = VICTOR_ACCESSORY_SERVICE_NAME });
    states.push_back({ .text = F("Identified"), .value = GlobalHelpers::toYesNoName(isIdentified) });
    states.push_back({ .text = F("Active"),     .value = toActiveName(activeState.value.uint8_value) });
    states.push_back({ .text = F("In Use"),     .value = toInUseName(inUseState.value.uint8_value) });
    states.push_back({ .text = F("Valve Type"), .value = toValveTypeName(valveTypeState.value.uint8_value) });
    states.push_back({ .text = F("Last Act"),   .value = GlobalHelpers::toMillisAgo(millis() - lastActMillis) });
    states.push_back({ .text = F("Act Name"),   .value = lastActName });
    states.push_back({ .text = F("Paired"),     .value = GlobalHelpers::toYesNoName(homekit_is_paired()) });
    states.push_back({ .text = F("Clients"),    .value = String(arduino_homekit_connected_clients_count()) });
    // buttons
    buttons.push_back({ .text = F("UnPair"), .value = F("UnPair") });
    buttons.push_back({ .text = F("Toggle"), .value = F("Toggle") });
  };
  appMain->webPortal->onServicePost = [](const String& value) {
    if (value == F("UnPair")) {
      homekit_server_reset();
      ESP.restart();
    } else if (value == F("Toggle")) {
      toggleActive(F("post"), connective);
    }
  };

  // setup homekit server
  hostName     = victorWiFi.getHostName();
  serialNumber = String(accessorySerialNumber.value.string_value) + "/" + victorWiFi.getHostId();
  accessoryNameInfo.value.string_value     = const_cast<char*>(hostName.c_str());
  accessorySerialNumber.value.string_value = const_cast<char*>(serialNumber.c_str());
  activeState.setter = [](const homekit_value_t value) { setActive(F("setter"), uint8ToActive(value.uint8_value), connective); };
  arduino_homekit_setup(&serverConfig);
  onAccessoryIdentify(identify);

  // button
  const auto buttonJson = new PinStorage("/button.json");
  const auto buttonPin = buttonJson->load();
  delete buttonJson;
  if (buttonPin != nullptr && buttonPin->enable) {
    button = new ActionButtonInterrupt(buttonPin);
    button->onAction = [](const ButtonAction action) {
      console.log()
        .bracket(F("button"))
        .section(F("action"), String(action));
      if (action == BUTTON_ACTION_PRESSED) {
        builtinLed.flash();
        toggleActive(F("button"), connective);
      } else if (action == BUTTON_ACTION_DOUBLE_PRESSED) {
        builtinLed.flash(500);
        const auto enable = victorWiFi.isLightSleepMode();
        victorWiFi.enableAP(enable); // toggle enabling ap
      } else if (action == BUTTON_ACTION_PRESSED_HOLD_L1) {
        ESP.restart();
      } else if (action == BUTTON_ACTION_PRESSED_HOLD_L2) {
        homekit_server_reset();
        ESP.restart();
      }
    };
  }

  // state sensor
  inUseSensor = new DigitalSensor("/inUse.json");
  inUseSensor->onStateChange = [](const bool state) {
    builtinLed.flash();
    if (state == true) {
      setInUse(F("sensor"), IN_USE, connective);
    }
  };
  notInUseSensor = new DigitalSensor("/notInUse.json");
  notInUseSensor->onStateChange = [](const bool state) {
    builtinLed.flash();
    if (state == true) {
      setInUse(F("sensor"), NOT_IN_USE, connective);
    }
  };

  // control output
  const auto motorLine1Json = new PinStorage("/motorLine1.json");
  const auto motorLine1Config = motorLine1Json->load();
  delete motorLine1Json;
  if (motorLine1Config != nullptr && motorLine1Config->enable) {
    motorLine1 = new DigitalOutput(motorLine1Config);
  }
  const auto motorLine2Json = new PinStorage("/motorLine2.json");
  const auto motorLine2Config = motorLine2Json->load();
  delete motorLine2Json;
  if (motorLine2Config != nullptr && motorLine2Config->enable) {
    motorLine2 = new DigitalOutput(motorLine2Config);
  }

  // storage
  storage = new DigitalStateStorage("/state.json");
  const auto state = storage->load();
  if (state != nullptr) {
    const auto savedState = state->save
      ? *state->currentValue
      : *state->defaultValue;
    setActive(F("setup"), boolToActive(savedState), false);
  } else {
    motorIdle();
  }

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
  // button
  if (button != nullptr) { button->loop(); }
  // sensor
  if (inUseSensor != nullptr) { inUseSensor->loop(); }
  if (notInUseSensor != nullptr) { notInUseSensor->loop(); }
}
