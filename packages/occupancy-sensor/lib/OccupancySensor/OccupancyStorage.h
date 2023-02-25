#ifndef OccupancyStorage_h
#define OccupancyStorage_h

#include <FileStorage.h>
#include <Pin/PinModels.h>

namespace Victor::Components {

  struct OccupancySetting {
    // sensor input pin
    PinConfig* sensor = nullptr;

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
