#ifndef VictorWeb_h
#define VictorWeb_h

#include <functional>
#include <vector>
#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#if VICTOR_FEATURES_RADIO
  #include "Storage/RadioStorage/RadioStorage.h"
#endif
#include "VictorOTA.h"
#include "VictorWifi.h"

#ifndef VICTOR_FILE_SIZE_LIMIT
#define VICTOR_FILE_SIZE_LIMIT 2048 // 2k
#endif

namespace Victor::Components {

  struct TextValueModel {
    String text;
    String value;
  };

  class VictorWeb {
   public:
    VictorWeb(int port = 80);
    virtual ~VictorWeb();
    void setup();
    void loop();
    // server events
    typedef std::function<void()> TServerEventHandler;
    TServerEventHandler onRequestStart = nullptr;
    TServerEventHandler onRequestEnd = nullptr;
    typedef std::function<void(DynamicJsonDocument& res)> TPageDataHandler;
    TPageDataHandler onPageData = nullptr;
    // radio events
    #if VICTOR_FEATURES_RADIO
      typedef std::function<void(uint8_t index)> TRadioEmitHandler;
      TRadioEmitHandler onRadioEmit = nullptr;
    #endif
    // service
    typedef std::function<void(std::vector<TextValueModel>& states, std::vector<TextValueModel>& buttons)> TServiceGetHandler;
    typedef std::function<void(const String& value)> TServicePostHandler;
    TServiceGetHandler onServiceGet = nullptr;
    TServicePostHandler onServicePost = nullptr;

   protected:
    ESP8266WebServer* _server = nullptr;
    ESP8266HTTPUpdateServer* _httpUpdater = nullptr;
    virtual void _registerHandlers();
    virtual void _solvePageTokens(String& html);
    virtual void _getPageData(DynamicJsonDocument& res);
    void _sendHtml(const String& html);
    void _sendJson(const DynamicJsonDocument& doc);
    void _dispatchRequestStart();
    void _dispatchRequestEnd();
    void _handleIndexPage();
    void _handleSystemStatus();
    void _handleSystemReset();
    void _handleFileSystem();
    void _handleFiles();
    void _handleFileGet();
    void _handleFileSave();
    void _handleFileDelete();
    void _handleWifi();
    void _handleWifiList();
    void _handleWifiJoin();
    void _handleWifiJoinStatus();
    void _handleWifiMode();
    void _handleWifiReset();
    void _handleOta();
    void _handleOtaFire();
    void _handleNotFound();
    #if VICTOR_FEATURES_RADIO
      void _handleRadioGet();
      void _handleRadioSave();
      void _handleRadioEmitGet();
      void _handleRadioEmitSave();
      void _handleRadioEmitSend();
      void _handleRadioRuleGet();
      void _handleRadioRuleSave();
      void _handleRadioCommandGet();
      void _handleRadioCommandSave();
    #endif
    void _handleServiceGet();
    void _handleServicePost();
  };
} // namespace Victor::Components

#endif // VictorWeb_h
