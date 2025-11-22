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

// format: uint8; HAP section 9.89; 0 = Dots, 1 = Arabic numerals
// homekit_characteristic_t accessoryLabelNS      = HOMEKIT_CHARACTERISTIC_(SERVICE_LABEL_NAMESPACE, 1);

homekit_service_t informationService = HOMEKIT_SERVICE_(ACCESSORY_INFORMATION,
  .primary = false,
  .characteristics = (homekit_characteristic_t*[]) {
    &accessoryManufacturer,
    &accessorySerialNumber,
    &accessoryModel,
    &accessoryVersion,
    &accessoryIdentify,
    &accessoryNameInfo,
    // &accessoryLabelNS,
    NULL,
  },
);

// format: uint8; HAP section 9.75; 0 = Single Press, 1 = Double Press, 2 = Long Press
homekit_characteristic_t eventState1 = HOMEKIT_CHARACTERISTIC_(PROGRAMMABLE_SWITCH_EVENT, 0);
// format: string; HAP section 9.62; maximum length 64
homekit_characteristic_t nameState1 = HOMEKIT_CHARACTERISTIC_(NAME, "Stateless Programmable Switch 1");
// format: uint8; HAP section 9.88; minimum value 1, step value 1
homekit_characteristic_t levelIndex1 = HOMEKIT_CHARACTERISTIC_(SERVICE_LABEL_INDEX, 1);

homekit_service_t stateService1 = HOMEKIT_SERVICE_(STATELESS_PROGRAMMABLE_SWITCH,
  .primary = false,
  .characteristics = (homekit_characteristic_t*[]) {
    &eventState1,
    &nameState1,
    &levelIndex1,
    NULL,
  },
);

// format: uint8; HAP section 9.75; 0 = Single Press, 1 = Double Press, 2 = Long Press
homekit_characteristic_t eventState2 = HOMEKIT_CHARACTERISTIC_(PROGRAMMABLE_SWITCH_EVENT, 0);
// format: string; HAP section 9.62; maximum length 64
homekit_characteristic_t nameState2 = HOMEKIT_CHARACTERISTIC_(NAME, "Stateless Programmable Switch 2");
// format: uint8; HAP section 9.88; minimum value 1, step value 1
homekit_characteristic_t levelIndex2 = HOMEKIT_CHARACTERISTIC_(SERVICE_LABEL_INDEX, 2);

homekit_service_t stateService2 = HOMEKIT_SERVICE_(STATELESS_PROGRAMMABLE_SWITCH,
  .primary = false,
  .characteristics = (homekit_characteristic_t*[]) {
    &eventState2,
    &nameState2,
    &levelIndex2,
    NULL,
  },
);

// format: uint8; HAP section 9.75; 0 = Single Press, 1 = Double Press, 2 = Long Press
homekit_characteristic_t eventState3 = HOMEKIT_CHARACTERISTIC_(PROGRAMMABLE_SWITCH_EVENT, 0);
// format: string; HAP section 9.62; maximum length 64
homekit_characteristic_t nameState3 = HOMEKIT_CHARACTERISTIC_(NAME, "Stateless Programmable Switch 3");
// format: uint8; HAP section 9.88; minimum value 1, step value 1
homekit_characteristic_t levelIndex3 = HOMEKIT_CHARACTERISTIC_(SERVICE_LABEL_INDEX, 3);

homekit_service_t stateService3 = HOMEKIT_SERVICE_(STATELESS_PROGRAMMABLE_SWITCH,
  .primary = false,
  .characteristics = (homekit_characteristic_t*[]) {
    &eventState3,
    &nameState3,
    &levelIndex3,
    NULL,
  },
);

homekit_accessory_t* accessories[] = {
  HOMEKIT_ACCESSORY(
    .id = 1,
    .category = homekit_accessory_category_programmable_switch,
    .services = (homekit_service_t*[]) {
      &informationService,
      &stateService1,
      &stateService2,
      &stateService3,
      NULL,
    },
  ),
  NULL,
};

homekit_server_config_t serverConfig = {
  .accessories = accessories,
  .password = VICTOR_ACCESSORY_SERVER_PASSWORD,
};
