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
homekit_characteristic_t accessoryManufacturer = HOMEKIT_CHARACTERISTIC_(MANUFACTURER, VICTOR_ACCESSORY_INFORMATION_MANUFACTURER);
homekit_characteristic_t accessorySerialNumber = HOMEKIT_CHARACTERISTIC_(SERIAL_NUMBER, VICTOR_ACCESSORY_INFORMATION_SERIAL_NUMBER);
homekit_characteristic_t accessoryModel        = HOMEKIT_CHARACTERISTIC_(MODEL, VICTOR_ACCESSORY_INFORMATION_MODEL);
homekit_characteristic_t accessoryVersion      = HOMEKIT_CHARACTERISTIC_(FIRMWARE_REVISION, VICTOR_FIRMWARE_VERSION);
homekit_characteristic_t accessoryIdentify     = HOMEKIT_CHARACTERISTIC_(IDENTIFY, accessoryIdentifyHandler);
homekit_characteristic_t accessoryNameInfo     = HOMEKIT_CHARACTERISTIC_(NAME, VICTOR_ACCESSORY_SERVICE_NAME); // change on setup

homekit_service_t informationService = HOMEKIT_SERVICE_(ACCESSORY_INFORMATION,
  .primary = false,
  .characteristics = (homekit_characteristic_t*[]) {
    &accessoryManufacturer,
    &accessorySerialNumber,
    &accessoryModel,
    &accessoryVersion,
    &accessoryIdentify,
    &accessoryNameInfo,
    NULL,
  },
);

// format: uint8; HAP section 9.50; 0 = Leak is not detected, 1 = Leak is detected
homekit_characteristic_t leakState = HOMEKIT_CHARACTERISTIC_(LEAK_DETECTED, false);
// format: float; HAP section 9.128; minimum 0, maximum 100, step 1, unit percentage
homekit_characteristic_t levelState = HOMEKIT_CHARACTERISTIC_(WATER_LEVEL, 0);
// format: bool; HAP section 9.96; true or false
homekit_characteristic_t activeState = HOMEKIT_CHARACTERISTIC_(STATUS_ACTIVE, false);
// format: string; HAP section 9.62; maximum length 64
homekit_characteristic_t nameState = HOMEKIT_CHARACTERISTIC_(NAME, "Leak Sensor");

homekit_service_t stateService = HOMEKIT_SERVICE_(LEAK_SENSOR,
  .primary = true,
  .characteristics = (homekit_characteristic_t*[]) {
    &leakState,
    &levelState,
    &activeState,
    &nameState,
    NULL,
  },
);

homekit_accessory_t* accessories[] = {
  HOMEKIT_ACCESSORY(
    .id = 1,
    .category = homekit_accessory_category_sensor,
    .services = (homekit_service_t*[]) {
      &informationService,
      &stateService,
      NULL,
    },
  ),
  NULL,
};

homekit_server_config_t serverConfig = {
  .accessories = accessories,
  .password = VICTOR_ACCESSORY_SERVER_PASSWORD,
};
