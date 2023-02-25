#include "OtaStorage.h"

namespace Victor::Components {

  OtaStorage::OtaStorage(const char* filePath) : FileStorage(filePath) {
    _maxSize = 256;
  }

  void OtaStorage::_serializeTo(const OtaSetting* model, DynamicJsonDocument& doc) {
    doc[F("remote")] = model->remote;
  }

  void OtaStorage::_deserializeFrom(OtaSetting* model, const DynamicJsonDocument& doc) {
    model->remote = String(doc[F("remote")]);
  }

} // namespace Victor::Components
