#include "PinStorage.h"

namespace Victor::Components {

  PinStorage::PinStorage(const char* filePath) : FileStorage(filePath) {
    _maxSize = 192;
  }

  PinStorage::~PinStorage() {}

  void PinStorage::_serialize(const PinConfig* model, DynamicJsonDocument& doc) {
    // pin
    const JsonArray pinArr = doc.createNestedArray(F("pin"));
    pinArr[0] = model->pin;
    pinArr[1] = model->trueValue;
    if (model->interruptMode > 0) {
      pinArr[2] = model->interruptMode;
    }
    // debounce
    doc[F("debounce")] = model->debounce;
    // enable
    doc[F("enable")] = model->enable ? 1 : 0;
  }

  void PinStorage::_deserialize(PinConfig* model, const DynamicJsonDocument& doc) {
    // pin
    const auto pinArr = doc[F("pin")];
    model->pin = pinArr[0];
    model->trueValue = pinArr[1];
    const auto interruptMode = pinArr[2];
    if (!interruptMode.isNull()) {
      model->interruptMode = interruptMode;
    }
    // debounce
    const auto debounce = doc[F("debounce")];
    if (!debounce.isNull()) {
      model->debounce = debounce;
    }
    // enable
    const auto enable = doc[F("enable")];
    if (!enable.isNull()) {
      model->enable = enable == 1;
    }
  }

} // namespace Victor::Components
