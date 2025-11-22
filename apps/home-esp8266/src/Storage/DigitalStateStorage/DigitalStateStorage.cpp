#include "DigitalStateStorage.h"

namespace Victor::Components {

  DigitalStateStorage::DigitalStateStorage(const char* filePath) : FileStorage(filePath) {
    _maxSize = 128;
  }

  void DigitalStateStorage::_serialize(const DigitalStateConfig* model, DynamicJsonDocument& doc) {
    doc[F("save")]         = model->save          ? 1 : 0;
    doc[F("defaultValue")] = *model->defaultValue ? 1 : 0;
    doc[F("currentValue")] = *model->currentValue ? 1 : 0;
  }

  void DigitalStateStorage::_deserialize(DigitalStateConfig* model, const DynamicJsonDocument& doc) {
    model->save         = doc[F("save")]                  == 1;
    model->defaultValue = new bool(doc[F("defaultValue")] == 1);
    model->currentValue = new bool(doc[F("currentValue")] == 1);
  }

} // namespace Victor::Components
