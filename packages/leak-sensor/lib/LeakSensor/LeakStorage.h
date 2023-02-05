#ifndef LeakStorage_h
#define LeakStorage_h

#include <FileStorage.h>

namespace Victor::Components {

  struct LeakSetting {
    // sensor input pin
    // 0~127 = gpio
    //    -1 = disabled
    int8_t sensorPin = -1; // (-128~127)
    // 0 = LOW
    // 1 = HIGH
    uint8_t sensorTrueValue = 0; // (0~255)

    // ms debounce time to avoid fast changes
    uint16_t debounce = 0; // (0~65535)
  };

  class LeakStorage : public FileStorage<LeakSetting> {
   public:
    LeakStorage(const char* filePath = "/leak.json");

   protected:
    void _serializeTo(const LeakSetting* model, DynamicJsonDocument& doc) override;
    void _deserializeFrom(LeakSetting* model, const DynamicJsonDocument& doc) override;
  };

  // global
  extern LeakStorage leakStorage;

} // namespace Victor::Components

#endif // LeakStorage_h
