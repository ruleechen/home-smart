#ifndef BinaryStateStorage_h
#define BinaryStateStorage_h

#include "FileStorage.h"

namespace Victor::Components {

  struct BinaryStateConfig {
    // initial value
    bool initOn = false;
    // save state or not
    bool save = false;
    // state value saved
    bool isOn = false;
  };

  class BinaryStateStorage : public FileStorage<BinaryStateConfig> {
   public:
    BinaryStateStorage(const char* filePath);

   protected:
    void _serialize(const BinaryStateConfig* model, DynamicJsonDocument& doc) override;
    void _deserialize(BinaryStateConfig* model, const DynamicJsonDocument& doc) override;
  };

} // namespace Victor::Components

#endif // BinaryStateStorage_h
