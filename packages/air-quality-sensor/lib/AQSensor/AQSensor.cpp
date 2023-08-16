#include "AQSensor.h"

namespace Victor::Components {

  AQSensor::AQSensor() {
    _sgp30 = new SGP30();
  }

  AQSensor::~AQSensor() {
    if (_measureInterval != nullptr) {
      free(_measureInterval);
      _measureInterval = nullptr;
    }
    if (_resetInterval != nullptr) {
      free(_resetInterval);
      _resetInterval = nullptr;
    }
    if (_storeInterval != nullptr) {
      free(_storeInterval);
      _storeInterval = nullptr;
    }
    if (_sgp30 != nullptr) {
      delete _sgp30;
      _sgp30 = nullptr;
    }
  }

  bool AQSensor::begin() {
    const auto found = _sgp30->begin();
    if (found) {
      _init();
    }
    return found;
  }

  // has to be called after every power-up or soft reset
  void AQSensor::_init() {
    const auto setting = climateStorage.load();
    if (setting != nullptr) {
      if (_measureInterval == nullptr && setting->aq->loopSeconds > 0) {
        _measureInterval = new IntervalOverAuto(setting->aq->loopSeconds * 1000);
      }
      if (_resetInterval == nullptr && setting->aq->resetHours > 0) {
        _resetInterval = new IntervalOverAuto(setting->aq->resetHours * 60 * 60 * 1000);
      }
      if (_storeInterval == nullptr && setting->aqBaseline->storeHours > 0) {
        _storeInterval = new IntervalOver(setting->aqBaseline->storeHours * 60 * 60 * 1000);
      }
    }
    _sgp30->initAirQuality();
    if (
      setting != nullptr &&
      setting->aqBaseline->load &&
      setting->aqBaseline->co2 > 0 &&
      setting->aqBaseline->tvoc > 0
    ) {
      _sgp30->setBaseline(
        setting->aqBaseline->co2,
        setting->aqBaseline->tvoc
      );
      console.log()
        .bracket(F("load"))
        .section(F("co2"), String(setting->aqBaseline->co2))
        .section(F("tvoc"), String(setting->aqBaseline->tvoc));
    }
  }

  void AQSensor::reset() {
    _sgp30->generalCallReset();
    _init();
  }

  MeasureState AQSensor::measure() {
    const auto now = millis();
    if (_measureInterval == nullptr || !_measureInterval->isOver(now)) {
      return MEASURE_SKIPPED;
    }
    if (_resetInterval != nullptr && _resetInterval->isOver(now)) {
      reset();
      return MEASURE_SKIPPED;
    }
    ESP.wdtFeed();
    const auto readSuccess = _sgp30->measureAirQuality() == SGP30_SUCCESS;
    if (
      readSuccess &&
      _storeInterval != nullptr &&
      _storeInterval->isOver(now)
    ) {
      if (_sgp30->getBaseline() == SGP30_SUCCESS) {
        _storeInterval->start(now);
        auto setting = climateStorage.load();
        if (setting != nullptr) {
          setting->aqBaseline->load = true; // once we have aq baseline generated, enable load for next boot
          setting->aqBaseline->co2 = _sgp30->baselineCO2;
          setting->aqBaseline->tvoc = _sgp30->baselineTVOC;
          climateStorage.save(setting);
        }
        console.log()
          .bracket(F("store"))
          .section(F("co2"), String(_sgp30->baselineCO2))
          .section(F("tvoc"), String(_sgp30->baselineTVOC));
      }
    }
    return readSuccess ? MEASURE_SUCCESS : MEASURE_FAILED;
  }

  float AQSensor::getCO2() {
    return _sgp30->CO2;
  }

  float AQSensor::getTVOC() {
    return _sgp30->TVOC;
  }

  void AQSensor::setRelHumidity(float relHumidity, float temperature) {
    auto absHumidity = getAbsoluteHumidity(relHumidity, temperature);
    auto sensHumidity = doubleToFixedPoint(absHumidity);
    _sgp30->setHumidity(sensHumidity);
  }

  double AQSensor::getAbsoluteHumidity(float relativeHumidity, float temperature) {
    double eSat = 6.11 * pow(10.0, (7.5 * temperature / (237.7 + temperature)));
    double vaporPressure = (relativeHumidity * eSat) / 100; // millibars
    double absHumidity = 1000 * vaporPressure * 100 / ((temperature + 273) * 461.5); // Ideal gas law with unit conversions
    return absHumidity;
  }

  uint16_t AQSensor::doubleToFixedPoint(double number) {
    int power = 1 << 8;
    double number2 = number * power;
    uint16_t value = floor(number2 + 0.5);
    return value;
  }

} // namespace Victor::Components
