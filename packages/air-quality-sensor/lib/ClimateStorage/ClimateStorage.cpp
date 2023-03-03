#include "ClimateStorage.h"

namespace Victor::Components {

  ClimateStorage::ClimateStorage(const char* filePath) : FileStorage(filePath) {
    _maxSize = 512;
  }

  void ClimateStorage::_serializeTo(const ClimateSetting* model, DynamicJsonDocument& doc) {
    // ht
    const JsonObject htObj = doc.createNestedObject(F("ht"));
    htObj[F("loop")]   = model->ht->loopSeconds;
    htObj[F("report")] = model->ht->reportSeconds;
    htObj[F("reset")]  = model->ht->resetHours;
    htObj[F("enable")] = model->ht->enable ? 1 : 0;
    htObj[F("h+")]     = model->ht->humidityRevise;
    htObj[F("t+")]     = model->ht->temperatureRevise;
    // aq
    const JsonObject aqObj = doc.createNestedObject(F("aq"));
    aqObj[F("loop")]   = model->aq->loopSeconds;
    aqObj[F("report")] = model->aq->reportSeconds;
    aqObj[F("reset")]  = model->aq->resetHours;
    aqObj[F("enable")] = model->aq->enable ? 1 : 0;
    aqObj[F("co2+")]   = model->aq->co2Revise;
    aqObj[F("tvoc+")]  = model->aq->tvocRevise;
    // aq baseline
    const JsonObject aqBaselineObj = doc.createNestedObject(F("aqBaseline"));
    aqBaselineObj[F("load")]  = model->aqBaseline->load ? 1 : 0;
    aqBaselineObj[F("store")] = model->aqBaseline->storeHours;
    aqBaselineObj[F("co2")]   = model->aqBaseline->co2;
    aqBaselineObj[F("tvoc")]  = model->aqBaseline->tvoc;
  }

  void ClimateStorage::_deserializeFrom(ClimateSetting* model, const DynamicJsonDocument& doc) {
    // ht
    const auto htObj = doc[F("ht")];
    model->ht = new HTConfig({
      // time
      {
        .loopSeconds   = htObj[F("loop")],
        .reportSeconds = htObj[F("report")],
        .resetHours    = htObj[F("reset")],
      },
      // enable
      .enable = htObj[F("enable")] == 1,
      // revise
      .humidityRevise    = htObj[F("h+")],
      .temperatureRevise = htObj[F("t+")],
    });
    // aq
    const auto aqObj = doc[F("aq")];
    model->aq = new AQConfig({
      // time
      {
        .loopSeconds   = aqObj[F("loop")],
        .reportSeconds = aqObj[F("report")],
        .resetHours    = aqObj[F("reset")],
      },
      // enable
      .enable = aqObj[F("enable")] == 1,
      // revise
      .co2Revise  = aqObj[F("co2+")],
      .tvocRevise = aqObj[F("tvoc+")],
    });
    // aq baseline
    const auto aqBaselineObj = doc[F("aqBaseline")];
    model->aqBaseline = new AQBaseline({
      .load       = aqBaselineObj[F("load")] == 1,
      .storeHours = aqBaselineObj[F("store")],
      .co2        = aqBaselineObj[F("co2")],
      .tvoc       = aqBaselineObj[F("tvoc")],
    });
  }

  // global
  ClimateStorage climateStorage;

} // namespace Victor::Components
