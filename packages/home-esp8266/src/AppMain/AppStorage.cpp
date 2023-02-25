#include "AppStorage.h"

namespace Victor::Components {

  AppStorage::AppStorage(const char* filePath) : FileStorage(filePath) {
    _maxSize = 256;
  }

  void AppStorage::_serializeTo(const AppSetting* model, DynamicJsonDocument& doc) {
    doc[F("name")]  = model->name;
    doc[F("brand")] = model->brand;
    doc[F("sleep")] = model->sleepMillis;
    // led
    const auto ledArr = doc.createNestedArray(F("led"));
    ledArr[0] = model->led->pin;
    ledArr[1] = model->led->onHigh  ? 1 : 0;
    ledArr[2] = model->led->enabled ? 1 : 0;
  }

  void AppStorage::_deserializeFrom(AppSetting* model, const DynamicJsonDocument& doc) {
    const char* name = doc[F("name")];
    model->name        = String(name);
    model->brand       = String(doc[F("brand")]);
    model->sleepMillis = doc[F("sleep")];
    // led
    const auto ledArr = doc[F("led")];
    model->led = new LedSetting({
      .pin     = ledArr[0],
      .onHigh  = ledArr[1] == 1,
      .enabled = ledArr[2] == 1,
    });
  }

  // global
  AppStorage appStorage;

} // namespace Victor::Components
