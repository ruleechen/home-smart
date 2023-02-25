#ifndef RadioStorage_h
#define RadioStorage_h

#include "FileStorage.h"
#include "RadioModels.h"

namespace Victor::Components {
  class RadioStorage : public FileStorage<RadioModel> {
   public:
    RadioStorage(const char* filePath = "/radio.json");
    void broadcast(RadioMessage* message);
    RadioMessage* getLastReceived() const;

   protected:
    RadioMessage* _lastReceived = nullptr;
    void _serializeTo(const RadioModel* model, DynamicJsonDocument& doc) override;
    void _deserializeFrom(RadioModel* model, const DynamicJsonDocument& doc) override;
  };

  // global
  extern RadioStorage radioStorage;

} // namespace Victor::Components

#endif // RadioStorage_h
