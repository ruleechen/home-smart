#include <homekit/homekit.h>
#include <homekit/characteristics.h>

typedef void (*IdentifyCallback)(homekit_value_t value);
IdentifyCallback _identifyCallback = NULL;
void onAccessoryIdentify(IdentifyCallback callback) {
  _identifyCallback = callback;
}
void accessoryIdentifyHandler(homekit_value_t value) {
  if (_identifyCallback != NULL) {
    _identifyCallback(value);
  }
}

// format: string; max length 64
homekit_characteristic_t accessorySerialNumber = HOMEKIT_CHARACTERISTIC_(SERIAL_NUMBER, VICTOR_ACCESSORY_INFORMATION_SERIAL_NUMBER); // change on setup
homekit_characteristic_t accessoryNameInfo     = HOMEKIT_CHARACTERISTIC_(NAME, VICTOR_ACCESSORY_SERVICE_NAME); // change on setup
// info service
homekit_service_t informationService = HOMEKIT_SERVICE_(ACCESSORY_INFORMATION,
  .characteristics = (homekit_characteristic_t*[]) {
    HOMEKIT_CHARACTERISTIC(MANUFACTURER, VICTOR_ACCESSORY_INFORMATION_MANUFACTURER),
    HOMEKIT_CHARACTERISTIC(MODEL, VICTOR_ACCESSORY_INFORMATION_MODEL),
    HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, VICTOR_FIRMWARE_VERSION),
    HOMEKIT_CHARACTERISTIC(IDENTIFY, accessoryIdentifyHandler),
    &accessorySerialNumber,
    &accessoryNameInfo,
    NULL,
  },
);

// format: float; min 0, max 100, step 0.1, unit celsius
homekit_characteristic_t temperatureState = HOMEKIT_CHARACTERISTIC_(CURRENT_TEMPERATURE, 0);
// format: bool; HAP section 9.96; true or false
homekit_characteristic_t temperatureActiveState = HOMEKIT_CHARACTERISTIC_(STATUS_ACTIVE, false);
// service
homekit_service_t temperatureService = HOMEKIT_SERVICE_(TEMPERATURE_SENSOR,
  .characteristics = (homekit_characteristic_t*[]) {
    &temperatureState,
    &temperatureActiveState,
    NULL,
  },
);

// format: float; min 0, max 100, step 1
homekit_characteristic_t humidityState = HOMEKIT_CHARACTERISTIC_(CURRENT_RELATIVE_HUMIDITY, 0);
// format: bool; HAP section 9.96; true or false
homekit_characteristic_t humidityActiveState = HOMEKIT_CHARACTERISTIC_(STATUS_ACTIVE, false);
// service
homekit_service_t humidityService = HOMEKIT_SERVICE_(HUMIDITY_SENSOR,
  .characteristics = (homekit_characteristic_t*[]) {
    &humidityState,
    &humidityActiveState,
    NULL,
  },
);

// format: float; HAP section 9.16; min 0, max 100000
homekit_characteristic_t carbonDioxideState = HOMEKIT_CHARACTERISTIC_(CARBON_DIOXIDE_DETECTED, 0);
// format: float; HAP section 9.16; min 0, max 100000
homekit_characteristic_t carbonDioxideLevelState = HOMEKIT_CHARACTERISTIC_(CARBON_DIOXIDE_LEVEL, 0);
// format: bool; HAP section 9.96; true or false
homekit_characteristic_t carbonDioxideActiveState = HOMEKIT_CHARACTERISTIC_(STATUS_ACTIVE, false);
// service
homekit_service_t carbonDioxideService = HOMEKIT_SERVICE_(CARBON_DIOXIDE_SENSOR,
  .characteristics = (homekit_characteristic_t*[]) {
    &carbonDioxideState,
    &carbonDioxideLevelState,
    &carbonDioxideActiveState,
    NULL,
  },
);

// format: float; HAP section 9.126; min 0, max 1000, step 1
homekit_characteristic_t vocDensityState = HOMEKIT_CHARACTERISTIC_(VOC_DENSITY, 0);
// format: uint8; HAP section 9.118; 0 = Unknown, 1 = Excellent, 2 = Good, 3 = Fair, 4 = Inferior, 5 = Poor
homekit_characteristic_t airQualityState = HOMEKIT_CHARACTERISTIC_(AIR_QUALITY, 0);
// format: bool; HAP section 9.96; true or false
homekit_characteristic_t airQualityActiveState = HOMEKIT_CHARACTERISTIC_(STATUS_ACTIVE, false);
// service
homekit_service_t airQualityService = HOMEKIT_SERVICE_(AIR_QUALITY_SENSOR,
  .primary = true,
  .characteristics = (homekit_characteristic_t*[]) {
    &airQualityState,
    &vocDensityState,
    &carbonDioxideLevelState,
    &airQualityActiveState,
    NULL,
  },
);

homekit_accessory_t* accessories[] = {
  HOMEKIT_ACCESSORY(
    .id = 1,
    .category = homekit_accessory_category_sensor,
    .services = (homekit_service_t*[]) {
      &informationService,
      &temperatureService,
      &humidityService,
      // &carbonDioxideService, // not compliance yet
      &airQualityService,
      NULL,
    },
  ),
  NULL,
};

homekit_server_config_t serverConfig = {
  .accessories = accessories,
  .password = VICTOR_ACCESSORY_SERVER_PASSWORD,
};
