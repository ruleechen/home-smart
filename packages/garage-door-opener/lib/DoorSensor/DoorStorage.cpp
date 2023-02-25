#include "DoorStorage.h"

namespace Victor::Components {

  DoorStorage::DoorStorage(const char* filePath) : FileStorage(filePath) {
    _maxSize = 256;
  }

  void DoorStorage::_serializeTo(const DoorSetting* model, DynamicJsonDocument& doc) {
    // door open
    const JsonArray openArr = doc.createNestedArray(F("open"));
    openArr[0] = model->doorOpen->pin;
    openArr[1] = model->doorOpen->trueValue;
    // door closed
    const JsonArray closedArr = doc.createNestedArray(F("closed"));
    closedArr[0] = model->doorClosed->pin;
    closedArr[1] = model->doorClosed->trueValue;
    // others
    doc[F("debounce")] = model->debounce;
    doc[F("autoStop")] = model->autoStop;
  }

  void DoorStorage::_deserializeFrom(DoorSetting* model, const DynamicJsonDocument& doc) {
    // door open
    const auto openArr = doc[F("open")];
    model->doorOpen = new PinConfig({
      .pin = openArr[0],
      .trueValue = openArr[1],
    });
    // door closed
    const auto closedArr = doc[F("closed")];
    model->doorClosed = new PinConfig({
      .pin = closedArr[0],
      .trueValue = closedArr[1],
    });
    // others
    model->debounce = doc[F("debounce")];
    model->autoStop = doc[F("autoStop")];
  }

  // global
  DoorStorage doorStorage;

} // namespace Victor::Components
