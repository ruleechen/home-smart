#include <homekit/homekit.h>
#include <homekit/characteristics.h>

void onAccessoryIdentify(homekit_value_t value) {
  printf("accessory identify\n");
}

// format: string; max length 64
homekit_characteristic_t accessoryManufacturer = HOMEKIT_CHARACTERISTIC_(MANUFACTURER, VICTOR_ACCESSORY_INFORMATION_MANUFACTURER);
homekit_characteristic_t accessorySerialNumber = HOMEKIT_CHARACTERISTIC_(SERIAL_NUMBER, VICTOR_ACCESSORY_INFORMATION_SERIAL_NUMBER);
homekit_characteristic_t accessoryModel        = HOMEKIT_CHARACTERISTIC_(MODEL, VICTOR_ACCESSORY_INFORMATION_MODEL);
homekit_characteristic_t accessoryVersion      = HOMEKIT_CHARACTERISTIC_(FIRMWARE_REVISION, VICTOR_FIRMWARE_VERSION);
homekit_characteristic_t accessoryIdentify     = HOMEKIT_CHARACTERISTIC_(IDENTIFY, onAccessoryIdentify);
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

// format: uint8; HAP section 9.118; 0 = opened, 1 = closed
homekit_characteristic_t targetDoorState = HOMEKIT_CHARACTERISTIC_(TARGET_DOOR_STATE, 1);
// format: uint8; HAP section 9.30; 0 = opened, 1 = closed, 2 = opening, 3 = closing, 4 = stopped not open or closed
homekit_characteristic_t currentDoorState = HOMEKIT_CHARACTERISTIC_(CURRENT_DOOR_STATE, 4);
// format: bool; HAP section 9.65; 0 = no obstruction, 1 = obstruction detected
homekit_characteristic_t obstructionState = HOMEKIT_CHARACTERISTIC_(OBSTRUCTION_DETECTED, 0);
// format: string; HAP section 9.62; maximum length 64
homekit_characteristic_t nameState = HOMEKIT_CHARACTERISTIC_(NAME, "Garage Door Opener");

homekit_service_t stateService = HOMEKIT_SERVICE_(GARAGE_DOOR_OPENER,
  .primary = true,
  .characteristics = (homekit_characteristic_t*[]) {
    &targetDoorState,
    &currentDoorState,
    &obstructionState,
    &nameState,
    NULL,
  },
);

homekit_accessory_t* accessories[] = {
  HOMEKIT_ACCESSORY(
    .id = 1,
    .category = homekit_accessory_category_garage,
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
