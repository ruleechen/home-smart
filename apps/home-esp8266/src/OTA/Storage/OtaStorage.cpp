#include "OtaStorage.h"

namespace Victor::Components {

  OtaStorage::OtaStorage(const char* filePath) : FileStorage(filePath) {
    _maxSize = 256;
  }

  void OtaStorage::_serialize(const OtaSetting* model, DynamicJsonDocument& doc) {
    doc[F("remote")] = model->remote;
  }

  void OtaStorage::_deserialize(OtaSetting* model, const DynamicJsonDocument& doc) {
    model->remote = String(doc[F("remote")]);
  }

} // namespace Victor::Components
