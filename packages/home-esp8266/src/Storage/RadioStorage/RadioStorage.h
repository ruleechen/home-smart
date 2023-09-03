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
    void _serialize(const RadioModel* model, DynamicJsonDocument& doc) override;
    void _deserialize(RadioModel* model, const DynamicJsonDocument& doc) override;
  };

  // global
  extern RadioStorage radioStorage;

} // namespace Victor::Components

#endif // RadioStorage_h
