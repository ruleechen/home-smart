#include "I2cStorage.h"

namespace Victor::Components {

  I2cStorage::I2cStorage(const char* filePath) : FileStorage(filePath) {
    _maxSize = 128;
  }

  void I2cStorage::_serializeTo(const I2cSetting* model, DynamicJsonDocument& doc) {
    doc[F("sda")] = model->sdaPin;
    doc[F("scl")] = model->sclPin;
    // enable pin
    const JsonArray enableArr = doc.createNestedArray(F("en"));
    enableArr[0] = model->enablePin;
    enableArr[1] = model->enableTrueValue;
  }

  void I2cStorage::_deserializeFrom(I2cSetting* model, const DynamicJsonDocument& doc) {
    model->sdaPin = doc[F("sda")];
    model->sclPin = doc[F("scl")];
    // enable pin
    const auto enableArr   = doc[F("en")];
    model->enablePin       = enableArr[0];
    model->enableTrueValue = enableArr[1];
  }

} // namespace Victor::Components
