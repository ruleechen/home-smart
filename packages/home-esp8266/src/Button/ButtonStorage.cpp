#include "ButtonStorage.h"

namespace Victor::Components {

  ButtonStorage::ButtonStorage(const char* filePath) : FileStorage(filePath) {
    _maxSize = 128;
  }

  void ButtonStorage::_serializeTo(const ButtonSetting* model, DynamicJsonDocument& doc) {
    const JsonArray buttonArr = doc.createNestedArray(F("button"));
    buttonArr[0] = model->buttonPin;
    buttonArr[1] = model->buttonTrueValue;
  }

  void ButtonStorage::_deserializeFrom(ButtonSetting* model, const DynamicJsonDocument& doc) {
    const auto buttonArr   = doc[F("button")];
    model->buttonPin       = buttonArr[0];
    model->buttonTrueValue = buttonArr[1];
  }

} // namespace Victor::Components
