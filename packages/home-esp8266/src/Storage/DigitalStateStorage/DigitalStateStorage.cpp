#include "DigitalStateStorage.h"

namespace Victor::Components {

  DigitalStateStorage::DigitalStateStorage(const char* filePath) : FileStorage(filePath) {
    _maxSize = 128;
  }

  void DigitalStateStorage::_serialize(const DigitalStateConfig* model, DynamicJsonDocument& doc) {
    doc[F("save")]         = model->save         ? 1 : 0;
    doc[F("initialValue")] = model->initialValue ? 1 : 0;
    doc[F("currentValue")] = model->currentValue ? 1 : 0;
  }

  void DigitalStateStorage::_deserialize(DigitalStateConfig* model, const DynamicJsonDocument& doc) {
    model->save         = doc[F("save")]         == 1;
    model->initialValue = doc[F("initialValue")] == 1;
    model->currentValue = doc[F("currentValue")] == 1;
  }

} // namespace Victor::Components
