#ifndef ClimateModels_h
#define ClimateModels_h

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
    HTConfig* ht = nullptr;
    AQConfig* aq = nullptr;
    AQBaseline* aqBaseline = nullptr;
  };

} // namespace Victor::Components

#endif // ClimateModels_h
