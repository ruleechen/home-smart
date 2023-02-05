#ifndef ClimateStorage_h
#define ClimateStorage_h

#include <FileStorage.h>

namespace Victor::Components {

  enum MeasureState {
    MEASURE_FAILED  = 0,
    MEASURE_SUCCESS = 1,
    MEASURE_SKIPPED = 2,
  };

  struct TimeConfig {
    // every seconds to read devices from i2c bus
    // 0 = disabled
    uint8_t loopSeconds = 0; // (0~255)

    // every seconds to report data
    // 0 = disabled
    uint8_t reportSeconds = 0; // (0~255)

    // every hours to soft reset devices on i2c bus
    // 0 = disabled
    uint8_t resetHours = 0;  // (0~255)
  };

  struct HTConfig : public TimeConfig {
    bool enable = false;
    float humidityRevise    = 0;
    float temperatureRevise = 0;
  };

  struct AQConfig : public TimeConfig {
    bool enable = false;
    float co2Revise = 0;
    float tvocRevise = 0;
  };

  struct AQBaseline {
    // load stored baseline on startup or not
    bool load = false;

    // hours to store baseline
    // 0 = disabled
    uint8_t storeHours = 0; // (0~255)

    // stored co2 baseline
    uint16_t co2 = 0; // (0~65535)

    // stored tvoc baseline
    uint16_t tvoc = 0; // (0~65535)
  };

  struct ClimateSetting {
    // button input pin
    // 0~127 = gpio
    //    -1 = disabled
    int8_t buttonPin = -1; // (-128~127)
    // 0 = LOW
    // 1 = HIGH
    uint8_t buttonTrueValue = 0; // (0~255)

    HTConfig* ht = nullptr;
    AQConfig* aq = nullptr;
    AQBaseline* aqBaseline = nullptr;
  };

  class ClimateStorage : public FileStorage<ClimateSetting> {
   public:
    ClimateStorage(const char* filePath = "/climate.json");

   protected:
    void _serializeTo(const ClimateSetting* model, DynamicJsonDocument& doc) override;
    void _deserializeFrom(ClimateSetting* model, const DynamicJsonDocument& doc) override;
  };

  // global
  extern ClimateStorage climateStorage;

} // namespace Victor::Components

#endif // ClimateStorage_h
