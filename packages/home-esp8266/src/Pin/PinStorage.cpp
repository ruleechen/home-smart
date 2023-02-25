#include "PinStorage.h"

namespace Victor::Components {

  PinStorage::PinStorage(const char* filePath) : FileStorage(filePath) {
    _maxSize = 128;
  }

  void PinStorage::_serializeTo(const PinConfig* model, DynamicJsonDocument& doc) {
    // pin
    const JsonArray pinArr = doc.createNestedArray(F("pin"));
    pinArr[0] = model->pin;
    pinArr[1] = model->trueValue;
    // enable
    doc[F("enable")] = model->enable ? 1 : 0;
  }

  void PinStorage::_deserializeFrom(PinConfig* model, const DynamicJsonDocument& doc) {
    // pin
    const auto pinArr = doc[F("pin")];
    model->pin = pinArr[0];
    model->trueValue  = pinArr[1];
    // enable
    model->enable = doc[F("enable")] == 1;
  }

} // namespace Victor::Components
