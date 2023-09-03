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

// format: uint8; HAP section 9.3; 0 = Inactive, 1 = Active
homekit_characteristic_t activeState = HOMEKIT_CHARACTERISTIC_(ACTIVE, 0);
// format: uint8; HAP section 9.48; 0 = Not in use, 1 = In use
homekit_characteristic_t inUseState = HOMEKIT_CHARACTERISTIC_(IN_USE, 0);
// format: uint8; HAP section 9.124; 0 = Generic valve, 1 = Irrigation, 2 = Shower head, 3 = Water faucet
homekit_characteristic_t valveTypeState = HOMEKIT_CHARACTERISTIC_(VALVE_TYPE, 0);
// format: string; HAP section 9.62; maximum length 64
homekit_characteristic_t nameState = HOMEKIT_CHARACTERISTIC_(NAME, "Valve");

homekit_service_t stateService = HOMEKIT_SERVICE_(VALVE,
  .primary = true,
  .characteristics = (homekit_characteristic_t*[]) {
    &activeState,
    &inUseState,
    &valveTypeState,
    &nameState,
    NULL,
  },
);

homekit_accessory_t* accessories[] = {
  HOMEKIT_ACCESSORY(
    .id = 1,
    .category = homekit_accessory_category_faucet,
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
