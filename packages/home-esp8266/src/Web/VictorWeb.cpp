#include "VictorWeb.h"

namespace Victor::Components {

  VictorWeb::VictorWeb(int port) {
    _server = new ESP8266WebServer(port);
    _httpUpdater = new ESP8266HTTPUpdateServer();
  }

  VictorWeb::~VictorWeb() {
    onRequestStart = nullptr;
    onRequestEnd = nullptr;
    onServiceGet = nullptr;
    onServicePost = nullptr;
    onPageData = nullptr;
    #if VICTOR_FEATURES_RADIO
      onRadioEmit = nullptr;
    #endif
    if (_server != nullptr) {
      _server->stop();
      delete _server;
      _server = nullptr;
    }
    if (_httpUpdater != nullptr) {
      delete _httpUpdater;
      _httpUpdater = nullptr;
    }
  }

  void VictorWeb::setup() {
    _registerHandlers();
    _httpUpdater->setup(_server, F("/update"));
    _server->begin();
  }

  void VictorWeb::loop() {
    _server->handleClient();
  }

  void VictorWeb::_registerHandlers() {
    // https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Cache-Control
    // max-age=600, no-cache, no-store, must-revalidate
    // "max-age=N" means N seconds
    _server->serveStatic("/w/", LittleFS, "/web/");
    _server->onNotFound(std::bind(&VictorWeb::_handleNotFound, this));
    _server->on(F("/"), HTTP_GET, std::bind(&VictorWeb::_handleIndexPage, this));
    _server->on(F("/system/status"), HTTP_GET, std::bind(&VictorWeb::_handleSystemStatus, this));
    _server->on(F("/system/reset"), HTTP_POST, std::bind(&VictorWeb::_handleSystemReset, this));
    _server->on(F("/fs"), HTTP_GET, std::bind(&VictorWeb::_handleFileSystem, this));
    _server->on(F("/files"), HTTP_GET, std::bind(&VictorWeb::_handleFiles, this));
    _server->on(F("/file"), HTTP_GET, std::bind(&VictorWeb::_handleFileGet, this));
    _server->on(F("/file"), HTTP_POST, std::bind(&VictorWeb::_handleFileSave, this));
    _server->on(F("/file"), HTTP_DELETE, std::bind(&VictorWeb::_handleFileDelete, this));
    _server->on(F("/wifi"), HTTP_GET, std::bind(&VictorWeb::_handleWiFi, this));
    _server->on(F("/wifi/list"), HTTP_GET, std::bind(&VictorWeb::_handleWiFiList, this));
    _server->on(F("/wifi/join"), HTTP_POST, std::bind(&VictorWeb::_handleWiFiJoin, this));
    _server->on(F("/wifi/join/status"), HTTP_GET, std::bind(&VictorWeb::_handleWiFiJoinStatus, this));
    _server->on(F("/wifi/mode"), HTTP_POST, std::bind(&VictorWeb::_handleWiFiMode, this));
    _server->on(F("/wifi/reset"), HTTP_POST, std::bind(&VictorWeb::_handleWiFiReset, this));
    _server->on(F("/ota"), HTTP_GET, std::bind(&VictorWeb::_handleOta, this));
    _server->on(F("/ota/fire"), HTTP_POST, std::bind(&VictorWeb::_handleOtaFire, this));
    #if VICTOR_FEATURES_RADIO
      _server->on(F("/radio"), HTTP_GET, std::bind(&VictorWeb::_handleRadioGet, this));
      _server->on(F("/radio"), HTTP_POST, std::bind(&VictorWeb::_handleRadioSave, this));
      _server->on(F("/radio/emit"), HTTP_GET, std::bind(&VictorWeb::_handleRadioEmitGet, this));
      _server->on(F("/radio/emit"), HTTP_POST, std::bind(&VictorWeb::_handleRadioEmitSave, this));
      _server->on(F("/radio/emit/send"), HTTP_POST, std::bind(&VictorWeb::_handleRadioEmitSend, this));
      _server->on(F("/radio/rule"), HTTP_GET, std::bind(&VictorWeb::_handleRadioRuleGet, this));
      _server->on(F("/radio/rule"), HTTP_POST, std::bind(&VictorWeb::_handleRadioRuleSave, this));
      _server->on(F("/radio/command"), HTTP_GET, std::bind(&VictorWeb::_handleRadioCommandGet, this));
      _server->on(F("/radio/command"), HTTP_POST, std::bind(&VictorWeb::_handleRadioCommandSave, this));
    #endif
    _server->on(F("/service/get"), HTTP_GET, std::bind(&VictorWeb::_handleServiceGet, this));
    _server->on(F("/service/post"), HTTP_POST, std::bind(&VictorWeb::_handleServicePost, this));
  }

  void VictorWeb::_solvePageTokens(String& html) {
    DynamicJsonDocument doc(512);
    _getPageData(doc);
    String json;
    serializeJson(doc, json);
    // replace
    html.replace(F("{json}"), json);
    html.replace(F("{title}"), VICTOR_FIRMWARE_SERVICE);
    html.replace(F("{timestamp}"), String(UNIX_TIME));
  }

  void VictorWeb::_getPageData(DynamicJsonDocument& res) {
    res[F("unixTime")] = UNIX_TIME;
    res[F("firmwareManufacturer")] = VICTOR_FIRMWARE_MANUFACTURER;
    res[F("firmwareService")] = VICTOR_FIRMWARE_SERVICE;
    res[F("firmwareVersion")] = VICTOR_FIRMWARE_VERSION;
    res[F("maxEditSize")] = VICTOR_FILE_SIZE_LIMIT;
    if (onPageData != nullptr) {
      onPageData(res);
    }
  }

  void VictorWeb::_sendHtml(const String& html) {
    _server->send(200, F("text/html"), html);
  }

  void VictorWeb::_sendJson(const DynamicJsonDocument& res) {
    String buf;
    serializeJson(res, buf);
    _server->send(200, F("application/json"), buf);
  }

  void VictorWeb::_dispatchRequestStart() {
    _server->sendHeader(F("Connection"), F("keep-alive")); // close / keep-alive
    // set cross origin
    _server->sendHeader(F("Access-Control-Allow-Origin"), F("*"));
    _server->sendHeader(F("Access-Control-Max-Age"), F("600")); // 10 minutes
    _server->sendHeader(F("Access-Control-Allow-Methods"), F("PUT,POST,GET,OPTIONS"));
    _server->sendHeader(F("Access-Control-Allow-Headers"), F("*"));
    // fire event
    if (onRequestStart != nullptr) {
      onRequestStart();
    }
  }

  void VictorWeb::_dispatchRequestEnd() {
    if (onRequestEnd != nullptr) {
      onRequestEnd();
    }
  }

  void VictorWeb::_handleNotFound() {
    _dispatchRequestStart();
    DynamicJsonDocument res(512);
    res[F("uri")] = _server->uri();
    res[F("err")] = F("notfound");
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void VictorWeb::_handleIndexPage() {
    _dispatchRequestStart();
    auto file = LittleFS.open(F("/web/index.htm"), "r");
    auto html = file.readString();
    file.close();
    _solvePageTokens(html);
    html.replace(F("{appendHead}"), F(""));
    _sendHtml(html);
    _dispatchRequestEnd();
  }

  void VictorWeb::_handleSystemStatus() {
    _dispatchRequestStart();
    // res
    DynamicJsonDocument res(1024);
    // status
    res[F("millis")] = millis(); // This number will overflow (go back to zero), after approximately 50 days.
    res[F("resetReason")] = ESP.getResetReason();
    res[F("freeStack")] = ESP.getFreeContStack();
    res[F("freeHeap")] = ESP.getFreeHeap();
    res[F("maxFreeBlockSize")] = ESP.getMaxFreeBlockSize();
    res[F("heapFragmentation")] = ESP.getHeapFragmentation();
    // hardware
    res[F("chipId")] = ESP.getChipId();
    res[F("cupFreqMHz")] = ESP.getCpuFreqMHz();
    res[F("flashId")] = ESP.getFlashChipId();
    res[F("flashSize")] = ESP.getFlashChipSize(); // Sketch thinks Flash RAM size is
    res[F("flashSizeReal")] = ESP.getFlashChipRealSize(); // Actual size based on chip Id
    res[F("flashSpeedMHz")] = ESP.getFlashChipSpeed() / 1000000;
    // software
    res[F("sketchMD5")] = ESP.getSketchMD5();
    res[F("sketchSize")] = ESP.getSketchSize();
    res[F("sketchFreeSpace")] = ESP.getFreeSketchSpace();
    res[F("sdkVersion")] = ESP.getSdkVersion();
    res[F("coreVersion")] = ESP.getCoreVersion();
    // end
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void VictorWeb::_handleSystemReset() {
    _dispatchRequestStart();
    // payload
    const auto payloadJson = _server->arg(F("plain"));
    DynamicJsonDocument payload(64);
    deserializeJson(payload, payloadJson);
    // read
    const auto values = String(payload[F("values")]);
    // action
    if (values.indexOf(F("1")) >= 0) {
      // sdk_system_restart();
      ESP.restart();
    }
    if (values.indexOf(F("2")) >= 0) {
      ESP.reset();
    }
    if (values.indexOf(F("3")) >= 0) {
      ESP.eraseConfig();
    }
    DynamicJsonDocument res(64);
    res[F("msg")] = F("success");
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void VictorWeb::_handleFileSystem() {
    _dispatchRequestStart();
    FSInfo fsInfo;
    DynamicJsonDocument res(512);
    if (LittleFS.info(fsInfo)) {
      res[F("totalBytes")] = fsInfo.totalBytes;
      res[F("usedBytes")] = fsInfo.usedBytes;
      res[F("maxPathLength")] = fsInfo.maxPathLength;
      res[F("maxOpenFiles")] = fsInfo.maxOpenFiles;
      res[F("blockSize")] = fsInfo.blockSize;
      res[F("pageSize")] = fsInfo.pageSize;
    } else {
      res[F("err")] = F("read fs info failed");
    }
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void VictorWeb::_handleFiles() {
    _dispatchRequestStart();
    DynamicJsonDocument res(1024);
    // load files
    // https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html
    const JsonArray filesArr = res.createNestedArray(F("files"));
    std::function<void(String)> loopFiles;
    loopFiles = [&](String path)->void {
      Dir dir = LittleFS.openDir(path);
      while(dir.next()) {
        const auto fullName = path + dir.fileName();
        if (dir.isDirectory()) {
          loopFiles(fullName + F("/"));
        } else if (dir.isFile()) {
          const JsonObject fileObj = filesArr.createNestedObject();
          fileObj[F("path")] = fullName;
          fileObj[F("size")] = dir.fileSize();
        }
      }
    };
    loopFiles(F("/"));
    // send
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void VictorWeb::_handleFileGet() {
    _dispatchRequestStart();
    DynamicJsonDocument res(VICTOR_FILE_SIZE_LIMIT);
    const auto path = _server->arg(F("path"));
    auto file = LittleFS.open(path, "r");
    if (file) {
      const auto name = String(file.name());
      const auto editable = !name.endsWith(F(".gz"));
      res[F("size")] = file.size();
      res[F("name")] = name;
      res[F("editable")] = editable;
      if (editable) {
        res[F("content")] = file.readString();
      }
      file.close();
    } else {
      res[F("err")] = F("failed to open file");
    }
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void VictorWeb::_handleFileSave() {
    _dispatchRequestStart();
    // payload
    const auto payloadJson = _server->arg(F("plain"));
    DynamicJsonDocument payload(2048); // 2k
    deserializeJson(payload, payloadJson);
    // write
    DynamicJsonDocument res(64);
    auto path = _server->arg(F("path"));
    const auto saveAs = String(payload[F("saveAs")]);
    if (saveAs.length() > 3) { // minimal path as "/f.n"
      path = saveAs;
    }
    auto file = LittleFS.open(path, "w+");
    if (file) {
      const char* content = payload[F("content")];
      file.write(content);
      file.close();
      res[F("msg")] = F("success");
    }
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void VictorWeb::_handleFileDelete() {
    _dispatchRequestStart();
    const auto path = _server->arg(F("path"));
    LittleFS.remove(path);
    DynamicJsonDocument res(64);
    res[F("msg")] = F("success");
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void VictorWeb::_handleWiFi() {
    _dispatchRequestStart();
    // wifi
    const auto mode = WiFi.getMode();
    const auto sleepMode = WiFi.getSleepMode();
    // station
    const auto staMacAddress = WiFi.macAddress();
    const auto isStaEnabled = ((mode & WIFI_STA) != 0);
    auto staAddress = String(F(""));
    if (isStaEnabled) {
      const IPAddress localIP = WiFi.localIP();
      if (localIP) {
        staAddress = localIP.toString();
      }
    }
    // access point
    const auto apMacAddress = WiFi.softAPmacAddress();
    const auto isApEnabled = ((mode & WIFI_AP) != 0);
    auto apAddress = String(F(""));
    if (isApEnabled) {
      const IPAddress apIP = WiFi.softAPIP();
      if (apIP) {
        apAddress = apIP.toString();
      }
    }
    // res
    DynamicJsonDocument res(512);
    // wifi
    res[F("hostName")] = victorWiFi.getHostName();
    res[F("mdns")] = victorWiFi.isMDNSRunning();
    res[F("mode")] = victorWiFi.modeName(mode);
    res[F("sleepMode")] = victorWiFi.sleepModeName(sleepMode);
    res[F("listenInterval")] = WiFi.getListenInterval();
    res[F("joined")] = WiFi.SSID();
    res[F("rssi")] = WiFi.RSSI();
    res[F("staAddress")] = staAddress;
    res[F("staMacAddress")] = staMacAddress;
    res[F("apAddress")] = apAddress;
    res[F("apMacAddress")] = apMacAddress;
    // end
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void VictorWeb::_handleWiFiList() {
    _dispatchRequestStart();
    DynamicJsonDocument res(1024);
    res[F("bssid")] = WiFi.BSSIDstr();
    const JsonArray apArr = res.createNestedArray(F("founds"));
    const auto count = WiFi.scanNetworks();
    for (int8_t i = 0; i < count; i++) {
      const JsonObject apObj = apArr.createNestedObject();
      apObj[F("bssid")] = WiFi.BSSIDstr(i); // Basic Service Set Identifiers (a MAC)
      apObj[F("ssid")] = WiFi.SSID(i); // Service Set Identifier
      apObj[F("rssi")] = WiFi.RSSI(i); // Received Signal Strength Indicator
      apObj[F("channel")] = WiFi.channel(i);
    }
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void VictorWeb::_handleWiFiJoin() {
    _dispatchRequestStart();
    // payload
    const auto payloadJson = _server->arg(F("plain"));
    DynamicJsonDocument payload(128);
    deserializeJson(payload, payloadJson);
    // read
    const auto bssid = String(payload[F("bssid")]);
    const auto ssid = String(payload[F("ssid")]);
    const auto pswd = String(payload[F("pswd")]);
    const int32_t channel = payload[F("channel")];
    // res
    DynamicJsonDocument res(64);
    if (ssid.isEmpty()) {
      res[F("msg")] = F("input ssid to join");
    } else {
      victorWiFi.join(ssid, pswd, channel, (uint8_t*)bssid.c_str());
      res[F("join")] = 1;
    }
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void VictorWeb::_handleWiFiJoinStatus() {
    _dispatchRequestStart();
    DynamicJsonDocument res(64);
    res[F("status")] = victorWiFi.status();
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void VictorWeb::_handleWiFiMode() {
    _dispatchRequestStart();
    // payload
    const auto payloadJson = _server->arg(F("plain"));
    DynamicJsonDocument payload(128);
    deserializeJson(payload, payloadJson);
    // read
    const uint8_t mode = payload[F("mode")];
    // set
    victorWiFi.setMode(WiFiMode_t(mode));
    // res
    DynamicJsonDocument res(64);
    res[F("msg")] = F("success");
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void VictorWeb::_handleWiFiReset() {
    _dispatchRequestStart();
    // set
    victorWiFi.reset();
    // res
    DynamicJsonDocument res(64);
    res[F("msg")] = F("success");
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void VictorWeb::_handleOta() {
    _dispatchRequestStart();
    DynamicJsonDocument res(512);
    res[F("flashSize")] = ESP.getFlashChipSize(); // Sketch thinks Flash RAM size is
    res[F("flashSizeReal")] = ESP.getFlashChipRealSize(); // Actual size based on chip Id
    res[F("sketchSize")] = ESP.getSketchSize();
    res[F("sketchFreeSpace")] = ESP.getFreeSketchSpace();
    res[F("otaVersion")] = victorOTA.getCurrentVersion();
    res[F("otaNewVersion")] = victorOTA.checkNewVersion();
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void VictorWeb::_handleOtaFire() {
    _dispatchRequestStart();
    // payload
    const auto payloadJson = _server->arg(F("plain"));
    DynamicJsonDocument payload(64);
    deserializeJson(payload, payloadJson);
    // read
    const auto version = String(payload[F("version")]);
    const auto otaType = String(payload[F("otaType")]);
    const auto type =
      otaType == F("all") ? OTA_ALL :
      otaType == F("fs") ? OTA_FS :
      otaType == F("sketch") ? OTA_SKETCH : OTA_SKETCH;
    // update
    victorOTA.update(version, type);
    // res
    DynamicJsonDocument res(64);
    res[F("msg")] = F("success");
    _sendJson(res);
    _dispatchRequestEnd();
  }

  #if VICTOR_FEATURES_RADIO
    void VictorWeb::_handleRadioGet() {
      _dispatchRequestStart();
      DynamicJsonDocument res(512);
      const auto setting = radioStorage.load();
      res[F("millis")] = millis();
      res[F("inputPin")] = setting != nullptr ? setting->inputPin : -1;
      res[F("outputPin")] = setting != nullptr ? setting->outputPin : -1;
      // last received
      const auto lastReceived = radioStorage.getLastReceived();
      if (lastReceived != nullptr) {
        const JsonObject lastReceivedObj = res.createNestedObject(F("lastReceived"));
        lastReceivedObj[F("value")] = lastReceived->value;
        lastReceivedObj[F("channel")] = lastReceived->channel;
        lastReceivedObj[F("timestamp")] = lastReceived->timestamp;
      }
      // end
      _sendJson(res);
      _dispatchRequestEnd();
    }

    void VictorWeb::_handleRadioSave() {
      _dispatchRequestStart();
      // payload
      const auto payloadJson = _server->arg(F("plain"));
      DynamicJsonDocument payload(64);
      deserializeJson(payload, payloadJson);
      // read
      const int8_t inputPin = payload[F("inputPin")];
      const int8_t outputPin = payload[F("outputPin")];
      // action
      auto setting = radioStorage.load();
      if (setting != nullptr) {
        setting->inputPin = inputPin;
        setting->outputPin = outputPin;
        radioStorage.save(setting);
      }
      // res
      DynamicJsonDocument res(64);
      res[F("msg")] = F("success");
      _sendJson(res);
      _dispatchRequestEnd();
    }

    void VictorWeb::_handleRadioEmitGet() {
      _dispatchRequestStart();
      DynamicJsonDocument res(1024 + 512);
      // emits
      const JsonArray emitArr = res.createNestedArray(F("emits"));
      const auto setting = radioStorage.load();
      if (setting != nullptr) {
        for (const auto emit : setting->emits) {
          const JsonObject emitObj = emitArr.createNestedObject();
          emitObj[F("name")] = emit->name;
          emitObj[F("value")] = emit->value;
          emitObj[F("channel")] = emit->channel;
          emitObj[F("press")] = emit->press;
        }
      }
      // last received
      const auto lastReceived = radioStorage.getLastReceived();
      if (lastReceived != nullptr) {
        const JsonObject lastReceivedObj = res.createNestedObject(F("lastReceived"));
        lastReceivedObj[F("value")] = lastReceived->value;
        lastReceivedObj[F("channel")] = lastReceived->channel;
      }
      // end
      _sendJson(res);
      _dispatchRequestEnd();
    }

    void VictorWeb::_handleRadioEmitSave() {
      _dispatchRequestStart();
      // payload
      const auto payloadJson = _server->arg(F("plain"));
      DynamicJsonDocument payload(1024 + 512);
      deserializeJson(payload, payloadJson);
      // read
      const auto emitItems = payload[F("emits")];
      // save
      auto setting = radioStorage.load();
      if (setting != nullptr) {
        for (auto emit : setting->emits) {
          delete emit;
        }
        setting->emits.clear();
        setting->emits.shrink_to_fit();
        for (size_t i = 0; i < emitItems.size(); i++) {
          const auto item = emitItems[i];
          setting->emits.push_back(new RadioEmit({
            .name = item[F("name")],
            .value = item[F("value")],
            .channel = item[F("channel")],
            .press = RadioPressState(item[F("press")]),
          }));
        }
        radioStorage.save(setting);
      }
      // res
      DynamicJsonDocument res(64);
      res[F("msg")] = F("success");
      _sendJson(res);
      _dispatchRequestEnd();
    }

    void VictorWeb::_handleRadioEmitSend() {
      _dispatchRequestEnd();
      // payload
      const auto payloadJson = _server->arg(F("plain"));
      DynamicJsonDocument payload(64);
      deserializeJson(payload, payloadJson);
      // read
      const uint8_t index = payload[F("index")];
      DynamicJsonDocument res(64);
      if (onRadioEmit != nullptr) {
        onRadioEmit(index);
        res[F("msg")] = F("success");
      } else {
        res[F("err")] = F("onRadioEmit is required");
      }
      _sendJson(res);
      _dispatchRequestEnd();
    }

    void VictorWeb::_handleRadioRuleGet() {
      _dispatchRequestStart();
      DynamicJsonDocument res(1024 + 512);
      // rules
      const JsonArray ruleArr = res.createNestedArray(F("rules"));
      const auto setting = radioStorage.load();
      if (setting != nullptr) {
        for (const auto rule : setting->rules) {
          const JsonObject ruleObj = ruleArr.createNestedObject();
          ruleObj[F("value")] = rule->value;
          ruleObj[F("channel")] = rule->channel;
          ruleObj[F("press")] = rule->press;
          ruleObj[F("action")] = rule->action;
        }
      }
      // last received
      const auto lastReceived = radioStorage.getLastReceived();
      if (lastReceived != nullptr) {
        const JsonObject lastReceivedObj = res.createNestedObject(F("lastReceived"));
        lastReceivedObj[F("value")] = lastReceived->value;
        lastReceivedObj[F("channel")] = lastReceived->channel;
      }
      // end
      _sendJson(res);
      _dispatchRequestEnd();
    }

    void VictorWeb::_handleRadioRuleSave() {
      _dispatchRequestStart();
      // payload
      const auto payloadJson = _server->arg(F("plain"));
      DynamicJsonDocument payload(1024 + 512);
      deserializeJson(payload, payloadJson);
      // read
      const auto ruleItems = payload[F("rules")];
      // save
      auto setting = radioStorage.load();
      if (setting != nullptr) {
        for (auto rule : setting->rules) {
          delete rule;
        }
        setting->rules.clear();
        setting->rules.shrink_to_fit();
        for (size_t i = 0; i < ruleItems.size(); i++) {
          const auto item = ruleItems[i];
          setting->rules.push_back(new RadioRule({
            .value = item[F("value")],
            .channel = item[F("channel")],
            .press = RadioPressState(item[F("press")]),
            .action = RadioAction(item[F("action")]),
          }));
        }
        radioStorage.save(setting);
      }
      // res
      DynamicJsonDocument res(64);
      res[F("msg")] = F("success");
      _sendJson(res);
      _dispatchRequestEnd();
    }

    void VictorWeb::_handleRadioCommandGet() {
      _dispatchRequestStart();
      DynamicJsonDocument res(1024 + 512);
      // commands
      const JsonArray commandArr = res.createNestedArray(F("commands"));
      const auto setting = radioStorage.load();
      if (setting != nullptr) {
        for (const auto command : setting->commands) {
          const JsonObject commandObj = commandArr.createNestedObject();
          commandObj[F("entry")] = command->entry;
          commandObj[F("action")] = command->action;
          commandObj[F("press")] = command->press;
        }
      }
      // end
      _sendJson(res);
      _dispatchRequestEnd();
    }

    void VictorWeb::_handleRadioCommandSave() {
      _dispatchRequestStart();
      // payload
      const auto payloadJson = _server->arg(F("plain"));
      DynamicJsonDocument payload(1024 + 512);
      deserializeJson(payload, payloadJson);
      // read
      const auto commandItems = payload[F("commands")];
      // save
      auto setting = radioStorage.load();
      if (setting != nullptr) {
        for (auto command : setting->commands) {
          delete command;
        }
        setting->commands.clear();
        setting->commands.shrink_to_fit();
        for (size_t i = 0; i < commandItems.size(); i++) {
          const auto item = commandItems[i];
          setting->commands.push_back(new RadioCommand({
            .entry = RadioCommandEntry(item[F("entry")]),
            .action = item[F("action")],
            .press = RadioPressState(item[F("press")]),
          }));
        }
        radioStorage.save(setting);
      }
      // res
      DynamicJsonDocument res(64);
      res[F("msg")] = F("success");
      _sendJson(res);
      _dispatchRequestEnd();
    }
  #endif

  void VictorWeb::_handleServiceGet() {
    _dispatchRequestStart();
    // read
    std::vector<TextValueModel> states = {
      { .text = F("Identify"), .value = victorWiFi.getHostName() }
    };
    std::vector<TextValueModel> buttons = {};
    if (onServiceGet != nullptr) {
      onServiceGet(states, buttons);
    }
    // write
    DynamicJsonDocument res(2048);
    const JsonArray statesArr = res.createNestedArray(F("states"));
    for (const auto& state : states) {
      const JsonArray stateArr = statesArr.createNestedArray();
      stateArr[0] = state.text;
      stateArr[1] = state.value;
    }
    const JsonArray buttonsArr = res.createNestedArray(F("buttons"));
    for (const auto& button : buttons) {
      const JsonArray buttonArr = buttonsArr.createNestedArray();
      buttonArr[0] = button.text;
      buttonArr[1] = button.value;
    }
    // res
    _sendJson(res);
    _dispatchRequestEnd();
  }

  void VictorWeb::_handleServicePost() {
    _dispatchRequestStart();
    // payload
    const auto payloadJson = _server->arg(F("plain"));
    DynamicJsonDocument payload(128);
    deserializeJson(payload, payloadJson);
    // read
    const auto value = payload[F("value")];
    // act
    DynamicJsonDocument res(512);
    if (onServicePost != nullptr) {
      onServicePost(value);
      res[F("msg")] = F("success");
    } else {
      res[F("err")] = F("onServicePost is required");
    }
    // res
    _sendJson(res);
    _dispatchRequestEnd();
  }

} // namespace Victor::Components
