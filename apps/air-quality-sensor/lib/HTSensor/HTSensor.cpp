#include "HTSensor.h"

namespace Victor::Components {

  HTSensor::HTSensor() {
    #if VICTOR_FEATURES_SENSOR_HT_AHT10
      _aht10 = new AHT10();
    #endif
    #if VICTOR_FEATURES_SENSOR_HT_SHT3X
      _sht30 = new SHT31();
    #endif
    const auto setting = climateStorage.load();
    if (setting != nullptr) {
      if (setting->ht->loopSeconds > 0) {
        _measureInterval = new IntervalOverAuto(setting->ht->loopSeconds * 1000);
      }
      if (setting->ht->resetHours > 0) {
        _resetInterval = new IntervalOverAuto(setting->ht->resetHours * 60 * 60 * 1000);
      }
    }
  }

  HTSensor::~HTSensor() {
    if (_measureInterval != nullptr) {
      delete _measureInterval;
      _measureInterval = nullptr;
    }
    if (_resetInterval != nullptr) {
      delete _resetInterval;
      _resetInterval = nullptr;
    }
    #if VICTOR_FEATURES_SENSOR_HT_AHT10
      if (_aht10 != nullptr) {
        delete _aht10;
        _aht10 = nullptr;
      }
    #endif
    #if VICTOR_FEATURES_SENSOR_HT_SHT3X
      if (_sht30 != nullptr) {
        delete _sht30;
        _sht30 = nullptr;
      }
    #endif
  }

  bool HTSensor::begin() {
    auto found = false;
    #if VICTOR_FEATURES_SENSOR_HT_AHT10
      found = _aht10->begin();
    #endif
    #if VICTOR_FEATURES_SENSOR_HT_SHT3X
      found = _sht30->begin();
    #endif
    return found;
  }

  void HTSensor::reset() {
    #if VICTOR_FEATURES_SENSOR_HT_AHT10
      if (_aht10 != nullptr) {
        _aht10->softReset();
      }
    #endif
    #if VICTOR_FEATURES_SENSOR_HT_SHT3X
      if (_sht30 != nullptr) {
        _sht30->reset();
      }
    #endif
  }

  MeasureState HTSensor::measure() {
    const auto now = millis();
    if (_measureInterval == nullptr || !_measureInterval->isOver(now)) {
      return MEASURE_SKIPPED;
    }
    if (_resetInterval != nullptr && _resetInterval->isOver(now)) {
      reset();
      return MEASURE_SKIPPED;
    }
    ESP.wdtFeed();
    auto readSuccess = false;
    #if VICTOR_FEATURES_SENSOR_HT_AHT10
      if (_aht10 != nullptr) {
        readSuccess = _aht10->readRawData() != AHT10_ERROR;
      }
    #endif
    #if VICTOR_FEATURES_SENSOR_HT_SHT3X
      if (_sht30 != nullptr) {
        readSuccess = _sht30->read();
      }
    #endif
    return readSuccess ? MEASURE_SUCCESS : MEASURE_FAILED;
  }

  float HTSensor::getHumidity() {
    #if VICTOR_FEATURES_SENSOR_HT_AHT10
      if (_aht10 != nullptr) {
        return _aht10->readHumidity(AHT10_USE_READ_DATA);
      }
    #endif
    #if VICTOR_FEATURES_SENSOR_HT_SHT3X
      if (_sht30 != nullptr) {
        return _sht30->getHumidity();
      }
    #endif
    return 0;
  }

  float HTSensor::getTemperature() {
    #if VICTOR_FEATURES_SENSOR_HT_AHT10
      if (_aht10 != nullptr) {
        return _aht10->readTemperature(AHT10_USE_READ_DATA);
      }
    #endif
    #if VICTOR_FEATURES_SENSOR_HT_SHT3X
      if (_sht30 != nullptr) {
        return _sht30->getTemperature();
      }
    #endif
    return 0;
  }

} // namespace Victor::Components
