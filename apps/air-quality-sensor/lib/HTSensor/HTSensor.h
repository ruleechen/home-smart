#ifndef HTSensor_h
#define HTSensor_h

#if VICTOR_FEATURES_SENSOR_HT_AHT10
  #include <AHT10.h>
#endif
#if VICTOR_FEATURES_SENSOR_HT_SHT3X
  #include <SHT31.h>
#endif
#include <Timer/IntervalOverAuto.h>
#include "ClimateStorage.h"

namespace Victor::Components {

  class HTSensor {
   public:
    HTSensor();
    ~HTSensor();
    bool begin();
    void reset();
    MeasureState measure();
    float getHumidity();
    float getTemperature();

   private:
    IntervalOverAuto* _measureInterval = nullptr;
    IntervalOverAuto* _resetInterval = nullptr;
    #if VICTOR_FEATURES_SENSOR_HT_AHT10
      AHT10* _aht10 = nullptr;
    #endif
    #if VICTOR_FEATURES_SENSOR_HT_SHT3X
      SHT31* _sht30 = nullptr;
    #endif
  };

} // namespace Victor::Components

#endif // HTSensor_h
