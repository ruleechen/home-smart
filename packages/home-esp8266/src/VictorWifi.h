#ifndef VictorWifi_h
#define VictorWifi_h

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include "Console.h"
#include "BuiltinLed.h"
#include "AppMain/AppStorage.h"
#include "WifiStorage/WifiStorage.h"

namespace Victor::Components {
  class VictorWifi {
   public:
    void setup(const char* settingFile = "/wifi.json");
    void reset();
    WiFiMode_t getMode() const;
    void setMode(WiFiMode_t mode);
    void enableAP(bool enable);
    void enableLightSleep(bool enable);
    bool isLightSleepMode();
    static String modeName(WiFiMode_t mode);
    static String sleepModeName(WiFiSleepType_t mode);
    void join(const String& ssid, const String& pswd = emptyString, int32_t channel = 0, uint8_t* bssid = nullptr);
    bool isConnected() const;
    bool isMDNSRunning() const;
    int8_t status();
    String getHostId();
    String getHostName();

   private:
    WifiStorage* _storage = nullptr;
    WiFiEventHandler _gotIPHandler = nullptr;
    WiFiEventHandler _disconnectedHandler = nullptr;
    void _handleStaGotIP(const WiFiEventStationModeGotIP& args);
    void _handleStaDisconnected(const WiFiEventStationModeDisconnected& args);
    static Console _log();
    String _joiningSsid;
    String _joiningPswd;
    bool _lightSleepEnabled = false;
  };

  // global
  extern VictorWifi victorWifi;

} // namespace Victor::Components

#endif // VictorWifi_h
