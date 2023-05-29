#include <Arduino.h>
#include <RCSwitch.h>
#include <arduino_homekit_server.h>

#include <AppMain/AppMain.h>

#include "DoorStorage.h"
#include "DoorSensor.h"

using namespace Victor;
using namespace Victor::Components;

extern "C" homekit_characteristic_t targetDoorState;
extern "C" homekit_characteristic_t currentDoorState;
extern "C" homekit_characteristic_t obstructionState;
extern "C" homekit_characteristic_t accessoryNameInfo;
extern "C" homekit_characteristic_t accessorySerialNumber;
extern "C" homekit_server_config_t serverConfig;

AppMain* appMain = nullptr;
bool connective = false;

RCSwitch rf = RCSwitch();
DoorSensor* doorSensor = nullptr;

String hostName;
String serialNumber;

String toDoorStateName(const uint8_t state) {
  return (
    state == 0 ? F("Open") :
    state == 1 ? F("Closed") :
    state == 2 ? F("Opening") :
    state == 3 ? F("Closing") :
    state == 4 ? F("Stopped") : F("Unknown")
  );
}

void emitDoorCommand(const DoorCommand command) {
  ESP.wdtFeed();
  switch (command) {
    case DOOR_COMMAND_OPEN:
      appMain->radioPortal->emit(F("open"));
      break;
    case DOOR_COMMAND_CLOSE:
      appMain->radioPortal->emit(F("close"));
      break;
    case DOOR_COMMAND_STOP:
      appMain->radioPortal->emit(F("stop"));
      break;
    default:
      break;
  }
}

void _homekitTargetDoorState(const TargetDoorState targetState, const bool notify) {
  targetDoorState.value.uint8_value = targetState;
  if (notify) {
    homekit_characteristic_notify(&targetDoorState, targetDoorState.value);
  }
}

void _homekitCurrentDoorState(const CurrentDoorState currentState, const bool notify) {
  currentDoorState.value.uint8_value = currentState;
  if (notify) {
    homekit_characteristic_notify(&currentDoorState, currentDoorState.value);
  }
}

void setTargetDoorState(const TargetDoorState targetState, const bool notify) {
  // update target state
  const auto currentState = CurrentDoorState(currentDoorState.value.uint8_value);
  _homekitTargetDoorState(targetState, notify);
  // emit radio
  if (targetState == TARGET_DOOR_STATE_OPEN) {
    if (currentState != CURRENT_DOOR_STATE_OPEN) {
      emitDoorCommand(DOOR_COMMAND_OPEN);
    }
  } else if (targetState == TARGET_DOOR_STATE_CLOSED) {
    if (currentState != CURRENT_DOOR_STATE_CLOSED) {
      emitDoorCommand(DOOR_COMMAND_CLOSE);
    }
  }
  // adjust current state
  if (currentState == CURRENT_DOOR_STATE_STOPPED) {
    if (targetState == TARGET_DOOR_STATE_OPEN) {
      _homekitCurrentDoorState(CURRENT_DOOR_STATE_OPENING, notify);
    } else if (targetState == TARGET_DOOR_STATE_CLOSED) {
      _homekitCurrentDoorState(CURRENT_DOOR_STATE_CLOSING, notify);
    }
  }
  // log
  console.log()
    .bracket(F("door"))
    .section(F("target"), toDoorStateName(targetState));
}

void setCurrentDoorState(const CurrentDoorState newState, const bool notify) {
  // update current state
  const auto currentState = CurrentDoorState(currentDoorState.value.uint8_value);
  _homekitCurrentDoorState(newState, notify);
  // adjust target state
  if (
    newState == CURRENT_DOOR_STATE_OPEN ||
    newState == CURRENT_DOOR_STATE_OPENING
  ) {
    _homekitTargetDoorState(TARGET_DOOR_STATE_OPEN, notify);
  } else if (
    newState == CURRENT_DOOR_STATE_CLOSED ||
    newState == CURRENT_DOOR_STATE_CLOSING
  ) {
    _homekitTargetDoorState(TARGET_DOOR_STATE_CLOSED, notify);
  } else if (
    newState == CURRENT_DOOR_STATE_STOPPED
  ) {
    _homekitTargetDoorState(TARGET_DOOR_STATE_OPEN, notify);
    // only when door is not OPEN and CLOSED then we can stop it
    if (
      currentState != CURRENT_DOOR_STATE_OPEN &&
      currentState != CURRENT_DOOR_STATE_CLOSED
    ) {
      emitDoorCommand(DOOR_COMMAND_STOP);
    }
  }
  // auto stop
  if (
    newState == CURRENT_DOOR_STATE_OPEN ||
    newState == CURRENT_DOOR_STATE_CLOSED
  ) {
    const auto setting = doorStorage.load();
    if (setting->autoStop > 0) {
      // pause some time before emit stop command to wait for door really stopped
      if (setting->autoStop > 1) { delay(setting->autoStop); }
      // emit stop command
      emitDoorCommand(DOOR_COMMAND_STOP);
    }
  }
  // led
  if (newState == CURRENT_DOOR_STATE_CLOSED) {
    builtinLed.turnOff(); // safe
  } else {
    builtinLed.turnOn(); // warn
  }
  // log
  console.log()
    .bracket(F("door"))
    .section(F("current"), toDoorStateName(newState));
}

void setup(void) {
  appMain = new AppMain();
  appMain->setup();

  // setup radio
  const auto radioJson = radioStorage.load();
  if (radioJson->inputPin > 0) {
    rf.enableReceive(radioJson->inputPin);
  }
  if (radioJson->outputPin > 0) {
    rf.enableTransmit(radioJson->outputPin);
  }
  appMain->radioPortal->onEmit = [](const RadioEmit* emit) {
    const auto value = emit->value.toInt();
    rf.setProtocol(emit->channel);
    rf.send(value, 24);
    builtinLed.flash();
    console.log()
      .bracket(F("radio"))
      .section(F("sent"), emit->value)
      .section(F("via channel"), String(emit->channel));
  };

  // setup web
  appMain->webPortal->onServiceGet = [](std::vector<TextValueModel>& states, std::vector<TextValueModel>& buttons) {
    // states
    states.push_back({ .text = F("Service"),     .value = VICTOR_ACCESSORY_SERVICE_NAME });
    states.push_back({ .text = F("Target"),      .value = toDoorStateName(targetDoorState.value.uint8_value) });
    states.push_back({ .text = F("Current"),     .value = toDoorStateName(currentDoorState.value.uint8_value) });
    states.push_back({ .text = F("Obstruction"), .value = GlobalHelpers::toYesNoName(obstructionState.value.bool_value) });
    states.push_back({ .text = F("Paired"),      .value = GlobalHelpers::toYesNoName(homekit_is_paired()) });
    states.push_back({ .text = F("Clients"),     .value = String(arduino_homekit_connected_clients_count()) });
    // buttons
    buttons.push_back({ .text = F("UnPair"),     .value = F("UnPair") });
    buttons.push_back({ .text = F("Door Open"),  .value = F("Open") });
    buttons.push_back({ .text = F("Door Close"), .value = F("Close") });
    buttons.push_back({ .text = F("Door Stop"),  .value = F("Stop") });
  };
  appMain->webPortal->onServicePost = [](const String& value) {
    if (value == F("UnPair")) {
      homekit_server_reset();
      ESP.restart();
    } else if (value == F("Open")) {
      setTargetDoorState(TARGET_DOOR_STATE_OPEN, connective);
    } else if (value == F("Close")) {
      setTargetDoorState(TARGET_DOOR_STATE_CLOSED, connective);
    } else if (value == F("Stop")) {
      setCurrentDoorState(CURRENT_DOOR_STATE_STOPPED, connective);
    }
  };

  // setup sensor
  doorSensor = new DoorSensor();
  doorSensor->onStateChange = [](const CurrentDoorState currentState) { setCurrentDoorState(currentState, connective); };
  setCurrentDoorState(doorSensor->readState(), false);

  // setup homekit server
  hostName     = victorWifi.getHostName();
  serialNumber = String(accessorySerialNumber.value.string_value) + "/" + victorWifi.getHostId();
  accessoryNameInfo.value.string_value     = const_cast<char*>(hostName.c_str());
  accessorySerialNumber.value.string_value = const_cast<char*>(serialNumber.c_str());
  targetDoorState.setter = [](const homekit_value_t value) { setTargetDoorState(TargetDoorState(value.uint8_value), connective); };
  arduino_homekit_setup(&serverConfig);

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
  doorSensor->loop();
  // loop radio
  if (rf.available()) {
    const auto value = String(rf.getReceivedValue());
    const auto channel = rf.getReceivedProtocol();
    appMain->radioPortal->receive(value, channel);
    builtinLed.flash();
    console.log()
      .bracket(F("radio"))
      .section(F("received"), value)
      .section(F("from channel"), String(channel));
    rf.resetAvailable();
  }
}
