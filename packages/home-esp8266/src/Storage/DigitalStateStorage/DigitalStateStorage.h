#ifndef DigitalStateStorage_h
#define DigitalStateStorage_h

#include "FileStorage.h"

namespace Victor::Components {

  struct DigitalStateConfig {
    // save state or not
    bool save = false;
    // initial value
    bool initialValue = false;
    // state value saved
    bool currentValue = false;
  };

  class DigitalStateStorage : public FileStorage<DigitalStateConfig> {
   public:
    DigitalStateStorage(const char* filePath);

   protected:
    void _serialize(const DigitalStateConfig* model, DynamicJsonDocument& doc) override;
    void _deserialize(DigitalStateConfig* model, const DynamicJsonDocument& doc) override;
  };

} // namespace Victor::Components

#endif // DigitalStateStorage_h
