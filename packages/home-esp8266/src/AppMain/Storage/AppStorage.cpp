#include "AppStorage.h"

namespace Victor::Components {

  AppStorage::AppStorage(const char* filePath) : FileStorage(filePath) {
    _maxSize = 256;
  }

  void AppStorage::_serialize(const AppSetting* model, DynamicJsonDocument& doc) {
    doc[F("name")]      = model->name;
    doc[F("brand")]     = model->brand;
    doc[F("sleep")]     = model->sleepMillis;
    doc[F("heartbeat")] = model->heartbeat;
  }

  void AppStorage::_deserialize(AppSetting* model, const DynamicJsonDocument& doc) {
    const char* name = doc[F("name")];
    model->name        = String(name);
    model->brand       = String(doc[F("brand")]);
    model->sleepMillis = doc[F("sleep")];
    // heartbeat
    const auto heartbeat = doc[F("heartbeat")];
    if (!heartbeat.isNull()) {
      model->heartbeat = heartbeat;
    }
  }

  // global
  AppStorage appStorage;

} // namespace Victor::Components
