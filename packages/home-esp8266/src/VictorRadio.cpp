#include "VictorRadio.h"

namespace Victor::Components {

  VictorRadio::VictorRadio() { }

  VictorRadio::~VictorRadio() {
    onEmit = nullptr;
    onAction = nullptr;
    onCommand = nullptr;
  }

  void VictorRadio::emit(const String& name) {
    const auto setting = radioStorage.load();
    if (setting->outputPin > -1) {
      for (const auto emit : setting->emits) {
        if (emit->name == name) {
          _handleEmit(emit);
          break;
        }
      }
    }
  }

  void VictorRadio::emit(uint8_t index) {
    const auto setting = radioStorage.load();
    if (setting->outputPin > -1 && index < setting->emits.size()) {
      const auto emit = setting->emits[index];
      _handleEmit(emit);
    }
  }

  void VictorRadio::_handleEmit(const RadioEmit* emit) {
    if (emit == nullptr) {
      return;
    }
    switch (emit->press) {
      case PRESS_STATE_SINGLE_PRESS: {
        _fireOnEmit(emit, 0);
        break;
      }
      case PRESS_STATE_DOUBLE_PRESS: {
        _fireOnEmit(emit, 300);
        break;
      }
      case PRESS_STATE_LONG_PRESS: {
        _fireOnEmit(emit, 2000);
        break;
      }
      case PRESS_STATE_AWAIT:
      default: {
        break;
      }
    }
  }

  void VictorRadio::_fireOnEmit(const RadioEmit* emit, const uint32_t nextDelay) {
    if (onEmit == nullptr || emit == nullptr) {
      return;
    }
    const auto clonedEmit = new RadioEmit({
      .name = GlobalHelpers::randomString(4), // radom id
      .value = emit->value,
      .channel = emit->channel,
      .press = emit->press,
    });
    onEmit(clonedEmit);
    if (nextDelay > 0) {
      delay(nextDelay);
      onEmit(clonedEmit);
    }
    delete clonedEmit;
  }

  void VictorRadio::receive(const String& value, const uint8_t channel) {
    const auto message = _parseMessage(value, channel);
    // press
    auto lastReceived = radioStorage.getLastReceived();
    const auto timespan = message->timestamp - (lastReceived != nullptr ? lastReceived->timestamp : 0);
    if (
      lastReceived == nullptr ||
      lastReceived->id != message->id  ||
      timespan > VICTOR_RADIO_RESET_THRESHOLD
    ) {
      lastReceived = nullptr;
      _lastPressState = PRESS_STATE_AWAIT;
    }
    if (
      _lastPressState != PRESS_STATE_SINGLE_PRESS &&
      (lastReceived == nullptr || lastReceived->value != message->value || lastReceived->channel != message->channel)
    ) {
      _handleReceived(message, PRESS_STATE_SINGLE_PRESS);
      radioStorage.broadcast(new RadioMessage({
        .id = message->id,
        .value = message->value,
        .channel = message->channel,
        .timestamp = message->timestamp,
      }));
    } else if (
      _lastPressState != PRESS_STATE_DOUBLE_PRESS &&
      timespan >= VICTOR_RADIO_DOUBLE_PRESS_MIN &&
      timespan < VICTOR_RADIO_DOUBLE_PRESS_MAX
    ) {
      _handleReceived(lastReceived, PRESS_STATE_DOUBLE_PRESS);
    } else if (
      _lastPressState != PRESS_STATE_LONG_PRESS &&
      timespan >= VICTOR_RADIO_LONG_PRESS_THRESHOLD
    ) {
      _handleReceived(lastReceived, PRESS_STATE_LONG_PRESS);
    }
    // release
    delete message;
  }

  void VictorRadio::_handleReceived(const RadioMessage* message, const RadioPressState press) {
    if (message == nullptr) {
      return;
    }
    // log states
    _lastPressState = press;
    console.log()
      .bracket(F("radio"))
      .section(F("detected pressed"), String(press));
    // check rules
    const auto setting = radioStorage.load();
    for (const auto rule : setting->rules) {
      if (
        rule->value == message->value &&
        rule->channel == message->channel &&
        rule->press == press
      ) {
        _proceedAction(rule);
      }
    }
    // check commands
    auto parsedCommand = _parseCommand(message->value);
    if (parsedCommand != nullptr) {
      for (const auto command : setting->commands) {
        if (
          command->entry == parsedCommand->entry &&
          command->action == parsedCommand->action &&
          command->press == press
        ) {
          _proceedCommand(parsedCommand);
          delete parsedCommand;
        }
      }
    }
  }

  void VictorRadio::_proceedAction(const RadioRule* rule) {
    if (rule == nullptr) {
      return;
    }
    if (onAction != nullptr) {
      const auto handled = onAction(rule);
      if (handled) {
        return;
      }
    }
    switch (rule->action) {
      case RADIO_ACTION_WIFI_STA: {
        victorWifi.setMode(WIFI_STA);
        break;
      }
      case RADIO_ACTION_WIFI_STA_AP: {
        victorWifi.setMode(WIFI_AP_STA);
        break;
      }
      case RADIO_ACTION_WIFI_RESET: {
        victorWifi.reset();
        break;
      }
      case RADIO_ACTION_ESP_RESTART: {
        ESP.restart();
        break;
      }
      default: {
        break;
      }
    }
  }

  void VictorRadio::_proceedCommand(const RadioCommandParsed* command) {
    if (command == nullptr) {
      return;
    }
    if (onCommand != nullptr) {
      const auto handled = onCommand(command);
      if (handled) {
        return;
      }
    }
    switch (command->entry) {
      case ENTRY_WIFI: {
        switch (command->action) {
          case ENTRY_WIFI_JOIN: {
            const auto credential = GlobalHelpers::splitString(command->parameters, F("/"));
            if (credential.size() == 2) {
              const auto ssid = credential[0];
              const auto pswd = credential[1];
              victorWifi.join(ssid, pswd, false);
            }
            break;
          }
          case ENTRY_WIFI_MODE: {
            const auto hasAP = command->parameters.indexOf(F("ap")) > -1;
            const auto hasSTA = command->parameters.indexOf(F("sta")) > -1;
            const auto isOff = command->parameters == F("off");
            if (hasAP && hasSTA) { victorWifi.setMode(WIFI_AP_STA); }
            else if (hasAP) { victorWifi.setMode(WIFI_AP); }
            else if (hasSTA) { victorWifi.setMode(WIFI_STA); }
            else if (isOff) { victorWifi.setMode(WIFI_OFF); }
            break;
          }
          case ENTRY_WIFI_RESET: {
            victorWifi.reset();
            break;
          }
          case ENTRY_WIFI_NONE:
          default: {
            break;
          }
        }
        break;
      }

      case ENTRY_APP: {
        switch (command->action) {
          case ENTRY_APP_NAME: {
            auto setting = appStorage.load();
            setting->name = command->parameters;
            appStorage.save(setting);
            break;
          }
          case ENTRY_APP_OTA: {
            const auto otaType =
              command->parameters == F("all") ? OTA_ALL :
              command->parameters == F("fs") ? OTA_FS :
              command->parameters == F("sketch") ? OTA_SKETCH : OTA_SKETCH;
            victorOTA.trigger(otaType);
            break;
          }
          case ENTRY_APP_NONE:
          default: {
            break;
          }
        }
        break;
      }

      case ENTRY_ESP: {
        switch (command->action) {
          case ENTRY_ESP_RESTART: {
            ESP.restart();
            break;
          }
          case ENTRY_ESP_NONE:
          default: {
            break;
          }
        }
        break;
      }

      case ENTRY_NONE:
      default: {
        break;
      }
    }
  }

  RadioMessage* VictorRadio::_parseMessage(const String& value, const uint8_t channel) {
    // read id
    auto idPart = String(F("NONE"));
    auto valuePart = String(value);
    if (value.indexOf(F("!")) == 4) {
      idPart = value.substring(0, 4);
      valuePart = value.substring(5);
    }
    // message
    const auto message = new RadioMessage({
      .id = idPart,
      .value = valuePart,
      .channel = channel,
      .timestamp = millis(),
    });
    // ret
    return message;
  }

  RadioCommandParsed* VictorRadio::_parseCommand(const String& value) {
    auto cloned = String(value); // clone
    cloned.replace(F(":"), F("-"));
    const auto parts = GlobalHelpers::splitString(cloned, F("-"));
    if (parts.size() < 2) {
      return nullptr;
    }
    auto command = new RadioCommandParsed();
    const auto entry = parts[0];
    const auto action = parts[1];
    if (entry == F("wifi")) {
      command->entry = ENTRY_WIFI;
      if (action == F("join")) {
        command->action = ENTRY_WIFI_JOIN;
      } else if (action == F("mode")) {
        command->action = ENTRY_WIFI_MODE;
      } else if (action == F("reset")) {
        command->action = ENTRY_WIFI_RESET;
      }
    } else if (entry == F("app")) {
      command->entry = ENTRY_APP;
      if (action == F("name")) {
        command->action = ENTRY_APP_NAME;
      } else if (action == F("ota")) {
        command->action = ENTRY_APP_OTA;
      }
    } else if (entry == F("esp")) {
      command->entry = ENTRY_ESP;
      if (action == F("restart")) {
        command->action = ENTRY_ESP_RESTART;
      }
    } else if (entry == F("boolean")) {
      command->entry = ENTRY_BOOLEAN;
      if (action == F("set")) {
        command->action = ENTRY_BOOLEAN_SET;
      } else if (action == F("toggle")) {
        command->action = ENTRY_BOOLEAN_TOGGLE;
      }
    }
    if (parts.size() >= 3) {
      const auto parameters = parts[2];
      command->parameters = parameters;
    }
    return command;
  }

} // namespace Victor::Components
