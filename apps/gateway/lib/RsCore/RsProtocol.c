#include "RsProtocol.h"

// Default Cyclic Redundancy Check
uint8_t rsGenerateCRC(uint8_t* payload) {
  return (
    payload[RS_PROTOCOL_INDEX_MODEL] ^
    payload[RS_PROTOCOL_INDEX_COMMAND] ^
    payload[RS_PROTOCOL_INDEX_DATA1] ^
    payload[RS_PROTOCOL_INDEX_DATA2] ^
    payload[RS_PROTOCOL_INDEX_DATA3] ^
    payload[RS_PROTOCOL_INDEX_DATA4] ^
    payload[RS_PROTOCOL_INDEX_DATA5] ^
    payload[RS_PROTOCOL_INDEX_DATA6] ^
    payload[RS_PROTOCOL_INDEX_DATA7] ^
    payload[RS_PROTOCOL_INDEX_DATA8]
  );
}

bool rsAuthenticate(uint8_t* payload) {
  return (
    payload[RS_PROTOCOL_INDEX_COMMAND] == RS_SERVER_COMMAND_AUTHENTICATE &&
    payload[RS_PROTOCOL_INDEX_DATA1]   == RS_PROTOCOL_AUTH_TOKEN1 &&
    payload[RS_PROTOCOL_INDEX_DATA2]   == RS_PROTOCOL_AUTH_TOKEN2 &&
    payload[RS_PROTOCOL_INDEX_DATA3]   == RS_PROTOCOL_AUTH_TOKEN3 &&
    payload[RS_PROTOCOL_INDEX_DATA4]   == RS_PROTOCOL_AUTH_TOKEN4
  );
}

void rsFillMessage(uint8_t* payload, uint8_t model, uint8_t command, uint8_t data1, uint8_t data2, uint8_t data3, uint8_t data4, uint8_t data5, uint8_t data6, uint8_t data7, uint8_t data8) {
  payload[RS_PROTOCOL_INDEX_HEAD1] = RS_PROTOCOL_HEAD1;
  payload[RS_PROTOCOL_INDEX_HEAD2] = RS_PROTOCOL_HEAD2;
  payload[RS_PROTOCOL_INDEX_VERSION] = RS_PROTOCOL_VERSION;
  payload[RS_PROTOCOL_INDEX_MODEL] = model;
  payload[RS_PROTOCOL_INDEX_COMMAND] = command;
  payload[RS_PROTOCOL_INDEX_DATA1] = data1;
  payload[RS_PROTOCOL_INDEX_DATA2] = data2;
  payload[RS_PROTOCOL_INDEX_DATA3] = data3;
  payload[RS_PROTOCOL_INDEX_DATA4] = data4;
  payload[RS_PROTOCOL_INDEX_DATA5] = data5;
  payload[RS_PROTOCOL_INDEX_DATA6] = data6;
  payload[RS_PROTOCOL_INDEX_DATA7] = data7;
  payload[RS_PROTOCOL_INDEX_DATA8] = data8;
  payload[RS_PROTOCOL_INDEX_CRC] = rsGenerateCRC(payload);
  payload[RS_PROTOCOL_INDEX_TAIL] = RS_PROTOCOL_TAIL;
}

bool rsValidateMessage(uint8_t* payload, size_t length) {
  if (
    length == RS_PROTOCOL_BYTES &&
    payload[RS_PROTOCOL_INDEX_HEAD1] == RS_PROTOCOL_HEAD1 &&
    payload[RS_PROTOCOL_INDEX_HEAD2] == RS_PROTOCOL_HEAD2 &&
    payload[RS_PROTOCOL_INDEX_VERSION] == RS_PROTOCOL_VERSION &&
    payload[RS_PROTOCOL_INDEX_TAIL] == RS_PROTOCOL_TAIL
  ) {
    uint8_t CRC = rsGenerateCRC(payload);
    if (payload[RS_PROTOCOL_INDEX_CRC] == CRC) {
      return true;
    }
  }
  return false;
}
