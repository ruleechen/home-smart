#ifndef DigitalStateConfig_h
#define DigitalStateConfig_h

#include <Arduino.h>

namespace Victor::Components {

  struct DigitalStateConfig {
    // save state or not
    bool save = false;

    // initial value
    bool initialValue = false;

    // state value saved
    bool currentValue = false;
  };

} // namespace Victor::Components

#endif // DigitalStateConfig_h
