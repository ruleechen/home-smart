#include "VictorWiFi.h"

namespace Victor::Components {

  void VictorWiFi::setup(const char* settingFile) {
    _storage = new WiFiStorage(settingFile);

    setMode(WIFI_AP_STA);
    WiFi.persistent(true);
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);

    // IPAddress apIp(192, 168, 1, 33);
    // IPAddress apSubnet(255, 255, 255, 0);
    // WiFi.softAPConfig(apIp, apIp, apSubnet);

    const auto hostName = getHostName();
    WiFi.hostname(hostName); // name which is displayed on router
    WiFi.softAP(hostName); // name which is displayed on AP list

    // need to keep the event handler for prevent unsubscribed automatically
    // https://github.com/esp8266/Arduino/issues/2545
    _gotIPHandler = WiFi.onStationModeGotIP(std::bind(&VictorWiFi::_handleStaGotIP, this, std::placeholders::_1));
    _disconnectedHandler = WiFi.onStationModeDisconnected(std::bind(&VictorWiFi::_handleStaDisconnected, this, std::placeholders::_1));

    const auto setting = _storage->load();
    if (setting != nullptr) {
      WiFi.begin(setting->ssid, setting->pswd);
    } else {
      WiFi.begin();
    }

    builtinLed.twinkle();
    _log().section(F("begin"))
      .section(F("ssid"), WiFi.SSID())
      .section(F("host"), WiFi.getHostname())
      .section(F("ap"), WiFi.softAPSSID())
      .section(F("ap-ip"), WiFi.softAPIP().toString());
  }

  void VictorWiFi::reset() {
    // wifi_config_reset();
    WiFi.disconnect(true);
    setMode(WIFI_AP_STA);
    builtinLed.twinkle();
  }

  WiFiMode_t VictorWiFi::getMode() const {
    return WiFi.getMode();
  }

  void VictorWiFi::setMode(WiFiMode_t mode) {
    if (getMode() != mode) {
      WiFi.mode(mode);
      _log().section(F("mode"), modeName(mode));
      if (MDNS.isRunning()) {
        MDNS.notifyAPChange();
      }
    }
  }

  void VictorWiFi::enableAP(bool enable) {
    WiFi.enableAP(enable);
    enableLightSleep(!enable);
    const auto mode = getMode();
    _log().section(F("mode"), modeName(mode));
  }

  void VictorWiFi::enableLightSleep(bool enable) {
    _lightSleepEnabled = enable;
    if (enable) {
      const auto setting = _storage->load();
      const auto dtimMultiplier = setting != nullptr ? setting->dtimMultiplier : 5;
      WiFi.setSleepMode(WIFI_LIGHT_SLEEP, dtimMultiplier);
    } else {
      WiFi.setSleepMode(WIFI_MODEM_SLEEP);
    }
    const auto sleepMode = WiFi.getSleepMode();
    _log().section(F("sleepMode"), sleepModeName(sleepMode));
  }

  bool VictorWiFi::isConnective() {
    // no other logic here to keep it fast
    // as this is being called heavily by arduino loop
    return _connective;
  }

  bool VictorWiFi::isLightSleepMode() {
    // no other logic here to keep it fast
    // as this is being called heavily by arduino loop
    return _lightSleepEnabled;
  }

  String VictorWiFi::modeName(WiFiMode_t mode) {
    if (mode == WIFI_STA) {
      return F("STA");
    } else if (mode == WIFI_AP) {
      return F("AP");
    } else if (mode == WIFI_AP_STA) {
      return F("AP+STA");
    } else if (mode == WIFI_OFF) {
      return F("OFF");
    } else {
      return F("Unknown");
    }
  }

  String VictorWiFi::sleepModeName(WiFiSleepType_t mode) {
    if (mode == WIFI_LIGHT_SLEEP) {
      return F("Light");
    } else if (mode == WIFI_MODEM_SLEEP) {
      return F("Modem");
    } else if (mode == WIFI_NONE_SLEEP) {
      return F("None");
    } else {
      return F("Unknown");
    }
  }

  void VictorWiFi::join(const String& ssid, const String& pswd, int32_t channel, uint8_t* bssid) {
    _log().section(F("joining"), ssid);
    _joiningSsid = ssid;
    _joiningPswd = pswd;
    WiFi.begin(ssid, pswd, channel, bssid, true);
  }

  bool VictorWiFi::isConnected() const {
    return WiFi.isConnected();
  }

  bool VictorWiFi::isMDNSRunning() const {
    return MDNS.isRunning();
  }

  int8_t VictorWiFi::status() {
    const auto result = WiFi.status();
    _log().section(F("got status"), String(result));
    return result;
  }

  String VictorWiFi::getHostId() {
    auto id = WiFi.macAddress();
    id.replace(F(":"), F(""));
    id.toUpperCase();
    id = id.substring(id.length() - 6);
    return id;
  }

  String VictorWiFi::getHostName() {
    const auto setting = appStorage.load();
    auto service = (setting == nullptr || setting->brand.isEmpty())
      ? VICTOR_FIRMWARE_SERVICE
      : setting->brand;
    service.trim();
    service.replace(' ', '-');
    return service + F("-") + getHostId();
  }

  void VictorWiFi::_handleStaGotIP(const WiFiEventStationModeGotIP& args) {
    _connective = true;
    builtinLed.stop();
    _log().section(F("station")).section(F("got ip"), args.ip.toString());
    auto setting = _storage->load();
    if (_joiningSsid.isEmpty()) {
      // turn off AP only when it is not a new join
      if (setting == nullptr || setting->autoMode) {
        setMode(WIFI_STA);
        enableLightSleep(true);
      }
    } else if (setting != nullptr) {
      // save new wifi credential
      if (
        setting->ssid != _joiningSsid ||
        setting->pswd != _joiningPswd
      ) {
        setting->ssid = _joiningSsid;
        setting->pswd = _joiningPswd;
        _storage->save(setting);
      }
    }
  }

  void VictorWiFi::_handleStaDisconnected(const WiFiEventStationModeDisconnected& args) {
    _connective = false;
    builtinLed.twinkle();
    _log().section(F("station"), F("disconnected"));
    const auto setting = _storage->load();
    if (setting == nullptr || setting->autoMode) {
      setMode(WIFI_AP_STA);
      enableLightSleep(false);
    }
  }

  Console VictorWiFi::_log() {
    return console.log().bracket(F("wifi"));
  }

  // global
  VictorWiFi victorWiFi;

} // namespace Victor::Components
