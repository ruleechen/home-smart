#include "LeakStorage.h"

namespace Victor::Components {

  LeakStorage::LeakStorage(const char* filePath) : FileStorage(filePath) {
    _maxSize = 128;
  }

  void LeakStorage::_serializeTo(const LeakSetting* model, DynamicJsonDocument& doc) {
    // sensor
    const JsonArray sensorArr = doc.createNestedArray(F("sensor"));
    sensorArr[0] = model->sensorPin;
    sensorArr[1] = model->sensorTrueValue;
    // others
    doc[F("debounce")] = model->debounce;
  }

  void LeakStorage::_deserializeFrom(LeakSetting* model, const DynamicJsonDocument& doc) {
    // sensor
    const auto sensorArr   = doc[F("sensor")];
    model->sensorPin       = sensorArr[0];
    model->sensorTrueValue = sensorArr[1];
    // others
    model->debounce = doc[F("debounce")];
  }

  // global
  LeakStorage leakStorage;

} // namespace Victor::Components
