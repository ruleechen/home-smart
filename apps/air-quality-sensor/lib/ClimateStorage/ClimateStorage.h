#ifndef ClimateStorage_h
#define ClimateStorage_h

#include <Storage/FileStorage.h>
#include "ClimateModels.h"

namespace Victor::Components {

  class ClimateStorage : public FileStorage<ClimateSetting> {
   public:
    ClimateStorage(const char* filePath = "/climate.json");

   protected:
    void _serialize(const ClimateSetting* model, DynamicJsonDocument& doc) override;
    void _deserialize(ClimateSetting* model, const DynamicJsonDocument& doc) override;
  };

  // global
  extern ClimateStorage climateStorage;

} // namespace Victor::Components

#endif // ClimateStorage_h
