#include "VictorWifi.h"

namespace Victor::Components {

  void VictorWifi::setup(const char* settingFile) {
    _storage = new WifiStorage(settingFile);

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
    _gotIPHandler = WiFi.onStationModeGotIP(std::bind(&VictorWifi::_handleStaGotIP, this, std::placeholders::_1));
    _disconnectedHandler = WiFi.onStationModeDisconnected(std::bind(&VictorWifi::_handleStaDisconnected, this, std::placeholders::_1));

    const auto setting = _storage->load();
    WiFi.begin(setting->ssid, setting->pswd);

    builtinLed.twinkle();
    _log().section(F("begin"))
      .section(F("ssid"), WiFi.SSID())
      .section(F("host"), WiFi.getHostname())
      .section(F("ap"), WiFi.softAPSSID())
      .section(F("ap-ip"), WiFi.softAPIP().toString());
  }

  void VictorWifi::reset() {
    // wifi_config_reset();
    WiFi.disconnect(true);
    setMode(WIFI_AP_STA);
    builtinLed.twinkle();
  }

  WiFiMode_t VictorWifi::getMode() const {
    return WiFi.getMode();
  }

  void VictorWifi::setMode(WiFiMode_t mode) {
    if (getMode() != mode) {
      WiFi.mode(mode);
      _log().section(F("mode"), modeName(mode));
    }
  }

  void VictorWifi::enableAP(bool enable) {
    WiFi.enableAP(enable);
    enableLightSleep(!enable);
    const auto mode = getMode();
    _log().section(F("mode"), modeName(mode));
  }

  void VictorWifi::enableLightSleep(bool enable) {
    _lightSleepEnabled = enable;
    if (enable) {
      const auto setting = _storage->load();
      WiFi.setSleepMode(WIFI_LIGHT_SLEEP, setting->dtimMultiplier);
    } else {
      WiFi.setSleepMode(WIFI_MODEM_SLEEP);
    }
    const auto sleepMode = WiFi.getSleepMode();
    _log().section(F("sleepMode"), sleepModeName(sleepMode));
  }

  bool VictorWifi::isConnective() {
    // no other logic here to keep it fast
    // as this is being called heavily by arduino loop
    return _connective;
  }

  bool VictorWifi::isLightSleepMode() {
    // no other logic here to keep it fast
    // as this is being called heavily by arduino loop
    return _lightSleepEnabled;
  }

  String VictorWifi::modeName(WiFiMode_t mode) {
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

  String VictorWifi::sleepModeName(WiFiSleepType_t mode) {
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

  void VictorWifi::join(const String& ssid, const String& pswd, int32_t channel, uint8_t* bssid) {
    _log().section(F("joining"), ssid);
    _joiningSsid = ssid;
    _joiningPswd = pswd;
    WiFi.begin(ssid, pswd, channel, bssid, true);
  }

  bool VictorWifi::isConnected() const {
    return WiFi.isConnected();
  }

  bool VictorWifi::isMDNSRunning() const {
    return MDNS.isRunning();
  }

  int8_t VictorWifi::status() {
    const auto result = WiFi.status();
    _log().section(F("got status"), String(result));
    return result;
  }

  String VictorWifi::getHostId() {
    auto id = WiFi.macAddress();
    id.replace(F(":"), F(""));
    id.toUpperCase();
    id = id.substring(id.length() - 6);
    return id;
  }

  String VictorWifi::getHostName() {
    const auto setting = appStorage.load();
    auto service = setting->brand.isEmpty()
      ? VICTOR_FIRMWARE_SERVICE
      : setting->brand;
    service.trim();
    service.replace(' ', '-');
    return service + F("-") + getHostId();
  }

  void VictorWifi::_handleStaGotIP(const WiFiEventStationModeGotIP& args) {
    _connective = true;
    builtinLed.stop();
    _log().section(F("station")).section(F("got ip"), args.ip.toString());
    auto setting = _storage->load();
    if (_joiningSsid.isEmpty()) {
      // turn off AP only when it is not a new join
      if (setting->autoMode) {
        setMode(WIFI_STA);
        enableLightSleep(true);
      }
    } else {
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

  void VictorWifi::_handleStaDisconnected(const WiFiEventStationModeDisconnected& args) {
    _connective = false;
    builtinLed.twinkle();
    _log().section(F("station"), F("disconnected"));
    const auto setting = _storage->load();
    if (setting->autoMode) {
      setMode(WIFI_AP_STA);
      enableLightSleep(false);
    }
  }

  Console VictorWifi::_log() {
    return console.log().bracket(F("wifi"));
  }

  // global
  VictorWifi victorWifi;

} // namespace Victor::Components
