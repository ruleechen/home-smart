#include "I2cStorage.h"

namespace Victor::Components {

  I2cStorage::I2cStorage(const char* filePath) : FileStorage(filePath) {
    _maxSize = 128;
  }

  void I2cStorage::_serialize(const I2cSetting* model, DynamicJsonDocument& doc) {
    doc[F("sda")] = model->sdaPin;
    doc[F("scl")] = model->sclPin;
    // chip select pin
    if (model->chipSelect != nullptr) {
      const JsonArray enArr = doc.createNestedArray(F("en"));
      enArr[0] = model->chipSelect->pin;
      enArr[1] = model->chipSelect->trueValue;
    }
  }

  void I2cStorage::_deserialize(I2cSetting* model, const DynamicJsonDocument& doc) {
    model->sdaPin = doc[F("sda")];
    model->sclPin = doc[F("scl")];
    // chip select pin
    const auto enArr   = doc[F("en")];
    if (!enArr.isNull()) {
      model->chipSelect = new PinConfig({
        .pin = enArr[0],
        .trueValue = enArr[1],
      });
    }
  }

} // namespace Victor::Components
