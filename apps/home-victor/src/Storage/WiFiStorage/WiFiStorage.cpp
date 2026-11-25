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
    model->ssid           = String(doc[F("ssid")]);
    model->pswd           = String(doc[F("pswd")]);
    model->autoMode       = doc[F("auto")] == 1;
    model->dtimMultiplier = doc[F("dtim")];
  }

} // namespace Victor::Components
