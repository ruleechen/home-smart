#ifndef VictorBleModels_h
#define VictorBleModels_h

#include <Arduino.h>
extern "C" {
  #include "RsProtocol.h"
}

namespace Victor::Components {

  struct ServerCommand {
    uint8_t model   = RS_SERVER_MODEL_UNKNOWN;
    uint8_t command = RS_SERVER_COMMAND_UNKNOWN;
    uint8_t data1   = RS_SERVER_STATE_UNKNOWN;
    uint8_t data2   = RS_SERVER_STATE_UNKNOWN;
    uint8_t data3   = RS_SERVER_STATE_UNKNOWN;
    uint8_t data4   = RS_SERVER_STATE_UNKNOWN;
    uint8_t data5   = RS_SERVER_STATE_UNKNOWN;
    uint8_t data6   = RS_SERVER_STATE_UNKNOWN;
    uint8_t data7   = RS_SERVER_STATE_UNKNOWN;
    uint8_t data8   = RS_SERVER_STATE_UNKNOWN;
  };

} // namespace Victor::Components

#endif // VictorBleModels_h
