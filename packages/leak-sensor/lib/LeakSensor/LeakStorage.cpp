#include "LeakStorage.h"

namespace Victor::Components {

  LeakStorage::LeakStorage(const char* filePath) : FileStorage(filePath) {
    _maxSize = 128;
  }

  void LeakStorage::_serializeTo(const LeakSetting* model, DynamicJsonDocument& doc) {
    // sensor
    const JsonArray sensorArr = doc.createNestedArray(F("sensor"));
    sensorArr[0] = model->sensor->pin;
    sensorArr[1] = model->sensor->trueValue;
    // others
    doc[F("heartbeat")] = model->heartbeat;
    doc[F("debounce")]  = model->debounce;
  }

  void LeakStorage::_deserializeFrom(LeakSetting* model, const DynamicJsonDocument& doc) {
    // sensor
    const auto sensorArr   = doc[F("sensor")];
    model->sensor = new PinConfig({
      .pin = sensorArr[0],
      .trueValue = sensorArr[1],
    });
    // others
    model->heartbeat = doc[F("heartbeat")];
    model->debounce  = doc[F("debounce")];
  }

  // global
  LeakStorage leakStorage;

} // namespace Victor::Components
