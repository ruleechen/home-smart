#include "OutletInUse.h"

namespace Victor::Components {

  OutletInUse::OutletInUse() {
  }

  OutletInUse::~OutletInUse() {
    if (_debounce != nullptr) {
      free(_debounce);
      _debounce = nullptr;
    }
    onStateChange = nullptr;
  }

  bool OutletInUse::getState() {
    return true;
  }

  void OutletInUse::loop() {
  }

} // namespace Victor::Components
