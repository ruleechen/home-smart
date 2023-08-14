#include "VictorOTA.h"

namespace Victor::Components {

  void VictorOTA::setup(const char* settingFile) {
    _storage = new OtaStorage(settingFile);
    // ESPhttpUpdate.setAuthorization(user, password);
    ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
    // hook events
    ESPhttpUpdate.onStart(std::bind(&VictorOTA::_handleStart, this));
    ESPhttpUpdate.onEnd(std::bind(&VictorOTA::_handleEnd, this));
    ESPhttpUpdate.onProgress(std::bind(&VictorOTA::_handleProgress, this, std::placeholders::_1, std::placeholders::_2));
    ESPhttpUpdate.onError(std::bind(&VictorOTA::_handleError, this, std::placeholders::_1));
  }

  String VictorOTA::getCurrentVersion() const {
    return VICTOR_FIRMWARE_VERSION;
  }

  String VictorOTA::checkNewVersion() {
    return F("99.4.99");
  }

  void VictorOTA::update(const String& version, const OtaType type) {
    switch (type) {
      case OTA_ALL: {
        ESPhttpUpdate.rebootOnUpdate(false);
        _updateSketch();
        ESPhttpUpdate.rebootOnUpdate(true);
        _updateFileSystem();
        break;
      }
      case OTA_SKETCH: {
        ESPhttpUpdate.rebootOnUpdate(true);
        _updateSketch();
        break;
      }
      case OTA_FS: {
        ESPhttpUpdate.rebootOnUpdate(true);
        _updateFileSystem();
        break;
      }
      default: {
        break;
      }
    }
  }

  void VictorOTA::trigger(const OtaType type) {
    // versions
    auto currentVersion = getCurrentVersion();
    auto newVersion = checkNewVersion();
    // 21.3.10 --> 21310
    currentVersion.replace(F("."), F(""));
    newVersion.replace(F("."), F(""));
    // compare
    if (newVersion.toInt() > currentVersion.toInt()) {
      update(newVersion, type);
    }
  }

  void VictorOTA::_updateSketch() {
    WiFiClient client;
    const auto setting = _storage->load();
    const auto currentVersion = getCurrentVersion();
    ESPhttpUpdate.update(client, setting->remote, currentVersion);
  }

  void VictorOTA::_updateFileSystem() {
    WiFiClient client;
    const auto setting = _storage->load();
    const auto currentVersion = getCurrentVersion();
    ESPhttpUpdate.updateFS(client, setting->remote, currentVersion);
  }

  void VictorOTA::_handleStart() {
    _log().section(F("start updating"));
  }

  void VictorOTA::_handleEnd() {
    _log().section(F("update finished"));
  }

  void VictorOTA::_handleProgress(int progress, int total) {
    _log().section(F("progress"), GlobalHelpers::toPercentage(progress / (total / 100)));
  }

  void VictorOTA::_handleError(int error) {
    const auto message = ESPhttpUpdate.getLastErrorString();
    _log().bracket(String(error)).section(F("last error"), message);
  }

  Console VictorOTA::_log() {
    return console.log().bracket(F("ota"));
  }

  // global
  VictorOTA victorOTA;

} // namespace Victor::Components
