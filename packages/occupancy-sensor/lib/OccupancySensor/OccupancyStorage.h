#ifndef OccupancyStorage_h
#define OccupancyStorage_h

#include <FileStorage.h>

namespace Victor::Components {

  struct OccupancySetting {
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

  class OccupancyStorage : public FileStorage<OccupancySetting> {
   public:
    OccupancyStorage(const char* filePath = "/occupancy.json");

   protected:
    void _serializeTo(const OccupancySetting* model, DynamicJsonDocument& doc) override;
    void _deserializeFrom(OccupancySetting* model, const DynamicJsonDocument& doc) override;
  };

  // global
  extern OccupancyStorage occupancyStorage;

} // namespace Victor::Components

#endif // OccupancyStorage_h
