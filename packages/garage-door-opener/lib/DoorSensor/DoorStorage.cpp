#include "DoorStorage.h"

namespace Victor::Components {

  DoorStorage::DoorStorage(const char* filePath) : FileStorage(filePath) {
    _maxSize = 256;
  }

  void DoorStorage::_serializeTo(const DoorSetting* model, DynamicJsonDocument& doc) {
    // door open
    const JsonArray openArr = doc.createNestedArray(F("open"));
    openArr[0] = model->doorOpenPin;
    openArr[1] = model->doorOpenTrueValue;
    // door closed
    const JsonArray closedArr = doc.createNestedArray(F("closed"));
    closedArr[0] = model->doorClosedPin;
    closedArr[1] = model->doorClosedTrueValue;
    // others
    doc[F("debounce")] = model->debounce;
    doc[F("autoStop")] = model->autoStop;
  }

  void DoorStorage::_deserializeFrom(DoorSetting* model, const DynamicJsonDocument& doc) {
    // door open
    const auto openArr       = doc[F("open")];
    model->doorOpenPin       = openArr[0];
    model->doorOpenTrueValue = openArr[1];
    // door closed
    const auto closedArr       = doc[F("closed")];
    model->doorClosedPin       = closedArr[0];
    model->doorClosedTrueValue = closedArr[1];
    // others
    model->debounce = doc[F("debounce")];
    model->autoStop = doc[F("autoStop")];
  }

  // global
  DoorStorage doorStorage;

} // namespace Victor::Components
