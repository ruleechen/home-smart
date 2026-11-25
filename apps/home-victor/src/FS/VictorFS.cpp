#include "VictorFS.h"

namespace Victor {

  VictorFS::VictorFS() {}

  VictorFS::~VictorFS() {
    _fs = nullptr;
    _initialized = false;
  }

  bool VictorFS::begin(FS& fs) {
    _fs = &fs;
    _initialized = true;
    return true;
  }

  bool VictorFS::exists(const char* path) {
    if (!_initialized || _fs == nullptr) {
      return false;
    }
    return _fs->exists(path);
  }

  VictorFile VictorFS::open(const char* path, const char* mode) {
    if (!_initialized || _fs == nullptr) {
      return VictorFile(File());
    }
    return VictorFile(_fs->open(path, mode));
  }

  bool VictorFS::remove(const char* path) {
    if (!_initialized || _fs == nullptr) {
      return false;
    }
    return _fs->remove(path);
  }

  bool VictorFS::rename(const char* path, const char* newPath) {
    if (!_initialized || _fs == nullptr) {
      return false;
    }
    return _fs->rename(path, newPath);
  }

  bool VictorFS::isInitialized() const {
    return _initialized;
  }

  // global
  VictorFS victorFS;

} // namespace Victor
