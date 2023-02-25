#include "AppMain.h"

namespace Victor::Components {

  AppMain::AppMain() {}

  AppMain::~AppMain() {
    #if VICTOR_FEATURES_WEB
      if (webPortal != nullptr) {
        delete webPortal;
        webPortal = nullptr;
      }
    #endif
    #if VICTOR_FEATURES_RADIO
      if (radioPortal != nullptr) {
        delete radioPortal;
        radioPortal = nullptr;
      }
    #endif
  }

  void AppMain::setup() {
    console.begin(115200);

    if (!LittleFS.begin()) {
      console.error()
        .bracket(F("fs"))
        .section(F("mount failed"));
    }

    const auto appSetting = appStorage.load();
    _sleepMillis = appSetting->sleepMillis;

    builtinLed.setup(appSetting->led);
    builtinLed.turnOn();

    victorOTA.setup("/ota.json");
    victorWifi.setup("/wifi.json");

    #if VICTOR_FEATURES_WEB
      webPortal = new VictorWeb(80);
      webPortal->onRequestStart = []() { builtinLed.toggle(); };
      webPortal->onRequestEnd = []() { builtinLed.toggle(); };
      #if VICTOR_FEATURES_RADIO
        webPortal->onRadioEmit = [&](uint8_t index) { radioPortal->emit(index); };
        webPortal->onPageData = [&](DynamicJsonDocument& res) { res[F("hasRadio")] = (radioPortal != nullptr); };
      #endif
      webPortal->setup();
    #endif

    #if VICTOR_FEATURES_RADIO
      radioPortal = new VictorRadio();
    #endif
  }

  void AppMain::loop(int8_t sleepMode) {
    #if VICTOR_FEATURES_WEB
      webPortal->loop();
    #endif
    // sleep
    if (sleepMode == -1) {
      sleepMode = victorWifi.isLightSleepMode();
    }
    if (sleepMode && _sleepMillis > 0) {
      delay(_sleepMillis);
    }
  }

} // namespace Victor::Components
