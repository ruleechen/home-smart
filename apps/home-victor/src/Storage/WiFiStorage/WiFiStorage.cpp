#include "WiFiStorage.h"

namespace Victor::Components {

  WiFiStorage::WiFiStorage(const char* filePath) : FileStorage(filePath) {
    _maxSize = 128;
  }

  void WiFiStorage::_serialize(const WiFiSetting* model, DynamicJsonDocument& doc) {
    doc[F("ssid")] = model->ssid;
    doc[F("pswd")] = model->pswd;
    doc[F("auto")] = model->autoMode ? 1 : 0;
    doc[F("dtim")] = model->dtimMultiplier;
  }

  void WiFiStorage::_deserialize(WiFiSetting* model, const DynamicJsonDocument& doc) {
    model->ssid           = doc[F("ssid")].as<const char*>();
    model->pswd           = doc[F("pswd")].as<const char*>();
    model->autoMode       = doc[F("auto")] == 1;
    model->dtimMultiplier = doc[F("dtim")];
  }

} // namespace Victor::Components
