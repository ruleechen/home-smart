#include "OccupancyStorage.h"

namespace Victor::Components {

  OccupancyStorage::OccupancyStorage(const char* filePath) : FileStorage(filePath) {
    _maxSize = 128;
  }

  void OccupancyStorage::_serializeTo(const OccupancySetting* model, DynamicJsonDocument& doc) {
    // sensor
    const JsonArray sensorArr = doc.createNestedArray(F("sensor"));
    sensorArr[0] = model->sensor->pin;
    sensorArr[1] = model->sensor->trueValue;
    // others
    doc[F("debounce")] = model->debounce;
  }

  void OccupancyStorage::_deserializeFrom(OccupancySetting* model, const DynamicJsonDocument& doc) {
    // sensor
    const auto sensorArr   = doc[F("sensor")];
    model->sensor = new PinConfig({
      .pin = sensorArr[0],
      .trueValue = sensorArr[1],
    });
    // others
    model->debounce = doc[F("debounce")];
  }

  // global
  OccupancyStorage occupancyStorage;

} // namespace Victor::Components
