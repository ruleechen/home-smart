#include <Arduino.h>
#include <Wire.h>
#include <arduino_homekit_server.h>

#include <AppMain/AppMain.h>
#include <GlobalHelpers.h>
#include <Pin/PinStorage.h>
#include <I2cStorage/I2cStorage.h>
#include <Button/ActionButtonInterrupt.h>

#include "ClimateStorage.h"
#include "HTSensor.h"
#include "AQSensor.h"

using namespace Victor;
using namespace Victor::Components;

// temperature
extern "C" homekit_characteristic_t temperatureState;
extern "C" homekit_characteristic_t temperatureActiveState;
// humidity
extern "C" homekit_characteristic_t humidityState;
extern "C" homekit_characteristic_t humidityActiveState;
// carbon dioxide
extern "C" homekit_characteristic_t carbonDioxideState;
extern "C" homekit_characteristic_t carbonDioxideLevelState;
extern "C" homekit_characteristic_t carbonDioxideActiveState;
// air quality
extern "C" homekit_characteristic_t vocDensityState;
extern "C" homekit_characteristic_t airQualityState;
extern "C" homekit_characteristic_t airQualityActiveState;
// others
extern "C" homekit_characteristic_t accessoryNameInfo;
extern "C" homekit_characteristic_t accessorySerialNumber;
extern "C" homekit_server_config_t serverConfig;
extern "C" void onAccessoryIdentify(void (*callback)(const homekit_value_t value));

AppMain* appMain = nullptr;
ActionButtonInterrupt* button = nullptr;

IntervalOverAuto* htReportInt = nullptr;
IntervalOverAuto* aqReportInt = nullptr;
HTSensor* ht = nullptr;
AQSensor* aq = nullptr;

String hostName;
String serialNumber;

enum AirQuality {
  AIR_QUALITY_UNKNOWN   = 0,
  AIR_QUALITY_EXCELLENT = 1,
  AIR_QUALITY_GOOD      = 2,
  AIR_QUALITY_FAIR      = 3,
  AIR_QUALITY_INFERIOR  = 4,
  AIR_QUALITY_POOR      = 5,
};

String toAirQualityName(const uint8_t state) {
  return (
    state == AIR_QUALITY_EXCELLENT ? F("Excellent") :
    state == AIR_QUALITY_GOOD      ? F("Good") :
    state == AIR_QUALITY_FAIR      ? F("Fair") :
    state == AIR_QUALITY_INFERIOR  ? F("Inferior") :
    state == AIR_QUALITY_POOR      ? F("Poor") : F("Unknown")
  );
}

AirQuality toAirQuality(const float value) {
  // 0 ~ 49
  if (value <= 49) {
    return AIR_QUALITY_EXCELLENT;
  }
  // 50 ~ 99
  if (value <= 99) {
    return AIR_QUALITY_GOOD;
  }
  // 100 ~ 299
  if (value <= 299) {
    return AIR_QUALITY_FAIR;
  }
  // 300 ~ 599
  if (value <= 599) {
    return AIR_QUALITY_INFERIOR;
  }
  // 600 ~ 1000
  return AIR_QUALITY_POOR;
}

enum CarbonDioxide {
  CARBON_DIOXIDE_NORMAL = 0,
  CARBON_DIOXIDE_ABNORMAL = 1,
};

String toCarbonDioxideName(const uint8_t state) {
  return state == CARBON_DIOXIDE_NORMAL ? F("Normal") : F("Abnormal");
}

CarbonDioxide toCarbonDioxide(const float value) {
  return value <= 599 ? CARBON_DIOXIDE_NORMAL : CARBON_DIOXIDE_ABNORMAL;
}

float fixFloat(const float value, const float min, const float max, const int8_t decimals = 2) {
  const auto fixed = std::max<float>(min, std::min<float>(max, value));
  const auto multiplier = std::pow(10, decimals);
  const auto fixedInt = (int)(fixed * multiplier);
  return ((float)fixedInt) / multiplier;
}

void measureHT(const HTConfig* config, const bool connective, const unsigned long now) {
  const auto state = ht->measure();
  if (state == MEASURE_SKIPPED) { return; }
  const auto notify = connective && htReportInt != nullptr && htReportInt->isOver(now);
  const auto htOk = state == MEASURE_SUCCESS;
  temperatureActiveState.value.bool_value = htOk;
  if (notify) {
    homekit_characteristic_notify(&temperatureActiveState, temperatureActiveState.value);
  }
  humidityActiveState.value.bool_value = htOk;
  if (notify) {
    homekit_characteristic_notify(&humidityActiveState, humidityActiveState.value);
  }
  if (htOk) {
    auto temperature = ht->getTemperature();
    if (!isnan(temperature)) {
      temperature += config->temperatureRevise;
      const auto temperatureFixed = fixFloat(temperature, 0, 100); // 0~100
      temperatureState.value.float_value = temperatureFixed;
      if (notify) {
        homekit_characteristic_notify(&temperatureState, temperatureState.value);
      }
    }
    auto humidity = ht->getHumidity();
    if (!isnan(humidity)) {
      humidity += config->humidityRevise;
      const auto humidityFixed = fixFloat(humidity, 0, 100); // 0~100
      humidityState.value.float_value = humidityFixed;
      if (notify) {
        homekit_characteristic_notify(&humidityState, humidityState.value);
      }
    }
    console.log()
      .bracket(F("ht"))
      .section(F("h"), String(humidity))
      .section(F("t"), String(temperature))
      .section(F("notify"), GlobalHelpers::toYesNoName(notify));
    // write to AQ
    if (aq != nullptr) {
      aq->setRelHumidity(humidity, temperature);
    }
  }
}

void measureAQ(const AQConfig* config, const bool connective, const unsigned long now) {
  const auto state = aq->measure();
  if (state == MEASURE_SKIPPED) { return; }
  const auto notify = connective && aqReportInt != nullptr && aqReportInt->isOver(now);
  const auto aqOk = state == MEASURE_SUCCESS;
  airQualityActiveState.value.bool_value = aqOk;
  if (notify) {
    homekit_characteristic_notify(&airQualityActiveState, airQualityActiveState.value);
  }
  carbonDioxideActiveState.value.bool_value = aqOk;
  if (notify) {
    homekit_characteristic_notify(&carbonDioxideActiveState, carbonDioxideActiveState.value);
  }
  if (aqOk) {
    auto co2 = aq->getCO2();
    if (!isnan(co2)) {
      co2 += config->co2Revise;
      const auto co2Fixed = fixFloat(co2, 0, 100000); // 0~100000
      carbonDioxideLevelState.value.float_value = co2Fixed;
      if (notify) {
        homekit_characteristic_notify(&carbonDioxideLevelState, carbonDioxideLevelState.value);
      }
      const auto co2Enum = toCarbonDioxide(co2Fixed);
      carbonDioxideState.value.uint8_value = co2Enum;
      if (notify) {
        homekit_characteristic_notify(&carbonDioxideState, carbonDioxideState.value);
      }
    }
    auto tvoc = aq->getTVOC();
    if (!isnan(tvoc)) {
      tvoc += config->tvocRevise;
      const auto tvocFixed = fixFloat(tvoc, 0, 1000); // 0~1000
      vocDensityState.value.float_value = tvocFixed;
      if (notify) {
        homekit_characteristic_notify(&vocDensityState, vocDensityState.value);
      }
      const auto qualityEnum = toAirQuality(tvocFixed);
      airQualityState.value.uint8_value = qualityEnum;
      if (notify) {
        homekit_characteristic_notify(&airQualityState, airQualityState.value);
      }
    }
    console.log()
      .bracket(F("aq"))
      .section(F("co2"), String(co2))
      .section(F("tvoc"), String(tvoc))
      .section(F("notify"), GlobalHelpers::toYesNoName(notify));
  }
}

void setup(void) {
  appMain = new AppMain();
  appMain->setup();

  // setup web
  appMain->webPortal->onServiceGet = [](std::vector<TextValueModel>& states, std::vector<TextValueModel>& buttons) {
    // states
    states.push_back({ .text = F("Service"),      .value = VICTOR_ACCESSORY_SERVICE_NAME });
    states.push_back({ .text = F("Temperature"),  .value = String(temperatureState.value.float_value) + F("°C") });
    states.push_back({ .text = F("Humidity"),     .value = GlobalHelpers::toPercentage(humidityState.value.float_value) });
    states.push_back({ .text = F("eCO2 Level"),   .value = String(carbonDioxideLevelState.value.float_value) + F("ppm/㎥") });
    states.push_back({ .text = F("eCO2"),         .value = toCarbonDioxideName(carbonDioxideState.value.uint8_value) });
    states.push_back({ .text = F("TVOC Density"), .value = String(vocDensityState.value.float_value) + F("ppb/㎥") });
    states.push_back({ .text = F("Air Quality"),  .value = toAirQualityName(airQualityState.value.uint8_value) });
    states.push_back({ .text = F("Paired"),       .value = GlobalHelpers::toYesNoName(homekit_is_paired()) });
    states.push_back({ .text = F("Clients"),      .value = String(arduino_homekit_connected_clients_count()) });
    // buttons
    buttons.push_back({ .text = F("UnPair"), .value = F("UnPair") }); // UnPair HomeKit
    if (ht != nullptr) {
      buttons.push_back({ .text = F("Reset HT"), .value = F("ht") });  // Reset Humidity/Temperature
    }
    if (aq != nullptr) {
      buttons.push_back({ .text = F("Reset AQ"), .value = F("aq") });  // Reset Air Quality
    }
  };
  appMain->webPortal->onServicePost = [](const String& value) {
    if (value == F("UnPair")) {
      homekit_server_reset();
      ESP.restart();
    } else if (value == F("ht")) {
      ht->reset();
    } else if (value == F("aq")) {
      aq->reset();
    }
  };

  // setup homekit server
  hostName     = victorWifi.getHostName();
  serialNumber = String(accessorySerialNumber.value.string_value) + "/" + victorWifi.getHostId();
  accessoryNameInfo.value.string_value     = const_cast<char*>(hostName.c_str());
  accessorySerialNumber.value.string_value = const_cast<char*>(serialNumber.c_str());
  arduino_homekit_setup(&serverConfig);
  onAccessoryIdentify([](const homekit_value_t value) { builtinLed.toggle(); });

  // button
  const auto buttonJson = new PinStorage("/button.json");
  const auto buttonPin = buttonJson->load();
  if (buttonPin->enable) {
    button = new ActionButtonInterrupt(buttonPin);
    button->onAction = [](const ButtonAction action) {
      console.log()
        .bracket(F("button"))
        .section(F("action"), String(action));
      if (action == BUTTON_ACTION_PRESSED) {
        builtinLed.flash();
      } else if (action == BUTTON_ACTION_DOUBLE_PRESSED) {
        builtinLed.flash(500);
        const auto enable = victorWifi.isLightSleepMode();
        victorWifi.enableAP(enable); // toggle enabling ap
      } else if (action == BUTTON_ACTION_PRESSED_HOLD_L1) {
        ESP.restart();
      } else if (action == BUTTON_ACTION_PRESSED_HOLD_L2) {
        homekit_server_reset();
        ESP.restart();
      }
    };
  }

  // setup i2c
  const auto i2cStorage = new I2cStorage("/i2c.json");
  const auto i2c = i2cStorage->load();
  if (
    i2c->chipSelect != nullptr &&
    i2c->chipSelect->pin > -1
  ) {
    const auto csI2c = new DigitalOutput(i2c->chipSelect);
    csI2c->setValue(false);
    delay(200);
    csI2c->setValue(true);
    delay(200);
    delete csI2c;
  }
  Wire.begin(   // https://zhuanlan.zhihu.com/p/137568249
    i2c->sdaPin, // Inter-Integrated Circuit - Serial Data (I2C-SDA)
    i2c->sclPin  // Inter-Integrated Circuit - Serial Clock (I2C-SCL)
  );

  // setup ht sensor
  const auto climate = climateStorage.load();
  if (climate->ht->enable) {
    if (climate->ht->reportSeconds > 0) {
      const auto offsetFactor = random(1, 10);
      htReportInt = new IntervalOverAuto((climate->ht->reportSeconds + offsetFactor) * 1000);
    }
    ht = new HTSensor();
    if (!ht->begin()) {
      console.error()
        .bracket(F("ht"))
        .section(F("notfound"));
    }
  }

  // setup aq sensor
  if (climate->aq->enable) {
    if (climate->aq->reportSeconds > 0) {
      const auto offsetFactor = random(1, 10);
      aqReportInt = new IntervalOverAuto((climate->aq->reportSeconds + offsetFactor) * 1000);
    }
    aq = new AQSensor();
    if (!aq->begin()) {
      console.error()
        .bracket(F("aq"))
        .section(F("notfound"));
    }
  }

  // done
  console.log()
    .bracket(F("setup"))
    .section(F("complete"));
}

void loop(void) {
  arduino_homekit_loop();
  // loop sensor
  const auto now = millis();
  const auto climate = climateStorage.load();
  const auto isPaired = arduino_homekit_get_running_server()->paired;
  const auto connective = victorWifi.isConnective() && isPaired;
  const auto isLightSleep = victorWifi.isLightSleepMode() && isPaired;
  if (ht != nullptr) { measureHT(climate->ht, connective, now); }
  if (aq != nullptr) { measureAQ(climate->aq, connective, now); }
  // sleep
  appMain->loop(isLightSleep);
  // button
  if (button != nullptr) {
    button->loop();
  }
}
