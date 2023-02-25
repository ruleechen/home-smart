#ifndef BinaryStorage_h
#define BinaryStorage_h

#include "FileStorage.h"
#include "Pin/PinModels.h"

namespace Victor::Components {

  struct BinaryConfig {
    // input pin
    PinConfig* input = nullptr;

    // output pin
    PinConfig* output = nullptr;

    // output2 pin
    PinConfig* output2 = nullptr;

    // save output state or not
    bool saveOutput = false;
    // output state saved
    bool outputIsOn = false;
  };

  class BinaryStorage : public FileStorage<BinaryConfig> {
   public:
    BinaryStorage(const char* filePath);

   protected:
    void _serializeTo(const BinaryConfig* model, DynamicJsonDocument& doc) override;
    void _deserializeFrom(BinaryConfig* model, const DynamicJsonDocument& doc) override;
  };

} // namespace Victor::Components

#endif // BinaryStorage_h
