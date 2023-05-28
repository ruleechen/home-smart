#include "BinaryStateStorage.h"

namespace Victor::Components {

  BinaryStateStorage::BinaryStateStorage(const char* filePath) : FileStorage(filePath) {
    _maxSize = 128;
  }

  void BinaryStateStorage::_serialize(const BinaryStateConfig* model, DynamicJsonDocument& doc) {
    doc[F("initOn")] = model->initOn ? 1 : 0;
    doc[F("save")]   = model->save   ? 1 : 0;
    doc[F("isOn")]   = model->isOn   ? 1 : 0;
  }

  void BinaryStateStorage::_deserialize(BinaryStateConfig* model, const DynamicJsonDocument& doc) {
    model->initOn = doc[F("initOn")] == 1;
    model->save   = doc[F("save")]   == 1;
    model->isOn   = doc[F("isOn")]   == 1;
  }

} // namespace Victor::Components
