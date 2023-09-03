#ifndef OtaSetting_h
#define OtaSetting_h

#include <Arduino.h>

namespace Victor::Components {

  struct OtaSetting {
    // remote uri for query new Firmware/FileSystem image
    String remote;
  };

} // namespace Victor::Components

#endif // OtaSetting_h
