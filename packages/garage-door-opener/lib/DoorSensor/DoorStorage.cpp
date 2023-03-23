#include "DoorStorage.h"

namespace Victor::Components {

  DoorStorage::DoorStorage(const char* filePath) : FileStorage(filePath) {
    _maxSize = 128;
  }

  void DoorStorage::_serialize(const DoorSetting* model, DynamicJsonDocument& doc) {
    doc[F("debounce")] = model->debounce;
    doc[F("autoStop")] = model->autoStop;
  }

  void DoorStorage::_deserialize(DoorSetting* model, const DynamicJsonDocument& doc) {
    model->debounce = doc[F("debounce")];
    model->autoStop = doc[F("autoStop")];
  }

  // global
  DoorStorage doorStorage;

} // namespace Victor::Components
