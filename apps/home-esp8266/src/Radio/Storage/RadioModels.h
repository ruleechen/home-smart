#ifndef RadioModels_h
#define RadioModels_h

#include <vector>
#include <Arduino.h>

namespace Victor::Components {

  struct RadioMessage {
    String id;
    String value;
    uint8_t channel = 0; // (0~255)
    unsigned long timestamp = 0;
  };

  enum RadioAction {
    RADIO_ACTION_NONE        = 0,
    RADIO_ACTION_TRUE        = 1,
    RADIO_ACTION_FALSE       = 2,
    RADIO_ACTION_TOGGLE      = 3,
    RADIO_ACTION_WIFI_STA    = 4,
    RADIO_ACTION_WIFI_STA_AP = 5,
    RADIO_ACTION_WIFI_RESET  = 6,
    RADIO_ACTION_ESP_RESTART = 7,
  };

  enum RadioPressState {
    PRESS_STATE_AWAIT        = 0,
    PRESS_STATE_SINGLE_PRESS = 1,
    PRESS_STATE_DOUBLE_PRESS = 2,
    PRESS_STATE_LONG_PRESS   = 3,
  };

  struct RadioEmit {
    String name;
    String value;
    uint8_t channel = 0; // (0~255)
    RadioPressState press = PRESS_STATE_SINGLE_PRESS;
  };

  struct RadioRule {
    String value;
    uint8_t channel = 0; // (0~255)
    RadioPressState press = PRESS_STATE_SINGLE_PRESS;
    RadioAction action = RADIO_ACTION_NONE;
  };

  enum RadioCommandEntry {
    ENTRY_NONE    = 0,
    ENTRY_WIFI    = 1,
    ENTRY_APP     = 2,
    ENTRY_ESP     = 3,
    ENTRY_BOOLEAN = 4,
  };

  enum EntryWiFiAction {
    ENTRY_WIFI_NONE  = 0,
    ENTRY_WIFI_JOIN  = 1, // parameter: {ssid}/{password}
    ENTRY_WIFI_MODE  = 2, // parameter: off, sta, ap, ap-sta
    ENTRY_WIFI_RESET = 3, // parameter: NONE
  };

  enum EntryAppAction {
    ENTRY_APP_NONE = 0,
    ENTRY_APP_NAME = 1, // parameter: {name}
    ENTRY_APP_OTA  = 2, // parameter: fs, sketch(default)
  };

  enum EntryEspAction {
    ENTRY_ESP_NONE    = 0,
    ENTRY_ESP_RESTART = 1, // parameter: NONE
  };

  enum EntryBooleanAction {
    ENTRY_BOOLEAN_NONE   = 0,
    ENTRY_BOOLEAN_SET    = 1, // parameter: true, false
    ENTRY_BOOLEAN_TOGGLE = 2,
  };

  struct RadioCommand {
    RadioCommandEntry entry = ENTRY_NONE;
    int8_t action = -1; // (-128~127)
    RadioPressState press = PRESS_STATE_SINGLE_PRESS;
  };

  struct RadioCommandParsed :RadioCommand {
    String parameters;
  };

  struct RadioModel {
    // 0~127 = gpio
    //    -1 = disabled
    int8_t inputPin = -1; // (-128~127)
    // 0~127 = gpio
    //    -1 = disabled
    int8_t outputPin = -1; // (-128~127)
    std::vector<RadioEmit*> emits       = {};
    std::vector<RadioRule*> rules       = {};
    std::vector<RadioCommand*> commands = {};
  };

} // namespace Victor::Components

#endif // RadioModels_h
