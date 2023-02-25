#ifndef LeakStorage_h
#define LeakStorage_h

#include <FileStorage.h>
#include <Pin/PinModels.h>

namespace Victor::Components {

  struct LeakSetting {
    // sensor input pin
    PinConfig* sensor = nullptr;

    // ms heartbeat interval
    uint16_t heartbeat = 0; // (0~65535)

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
