#ifndef RsProtocol_h
#define RsProtocol_h

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

// bt
#define RS_PROTOCOL_MANUFACTURER_DATA "RULEESMART"
#define RS_PROTOCOL_MANUFACTURER_DATA_LENGTH 10

// value
#define RS_PROTOCOL_BYTES     16 // payload length (max bytes + 1)
#define RS_PROTOCOL_HEAD1   0x52 // R
#define RS_PROTOCOL_HEAD2   0x53 // S
#define RS_PROTOCOL_VERSION 0x31 // 1
#define RS_PROTOCOL_TAIL    0x7C // |
#define RS_PROTOCOL_UNKNOWN 0x7E // ~

// index
#define RS_PROTOCOL_INDEX_HEAD1    0 // uint8_t/RS_PROTOCOL_HEAD1
#define RS_PROTOCOL_INDEX_HEAD2    1 // uint8_t/RS_PROTOCOL_HEAD2
#define RS_PROTOCOL_INDEX_VERSION  2 // uint8_t/RS_PROTOCOL_VERSION
#define RS_PROTOCOL_INDEX_MODEL    3 // uint8_t/RsServerModelType
#define RS_PROTOCOL_INDEX_COMMAND  4 // uint8_t/RsServerCommandType
#define RS_PROTOCOL_INDEX_DATA1    5 // uint8_t/RsServerStateType/RS_PROTOCOL_AUTH_TOKEN1...
#define RS_PROTOCOL_INDEX_DATA2    6 // uint8_t/RsServerStateType/RS_PROTOCOL_AUTH_TOKEN1...
#define RS_PROTOCOL_INDEX_DATA3    7 // uint8_t/RsServerStateType/RS_PROTOCOL_AUTH_TOKEN1...
#define RS_PROTOCOL_INDEX_DATA4    8 // uint8_t/RsServerStateType/RS_PROTOCOL_AUTH_TOKEN1...
#define RS_PROTOCOL_INDEX_DATA5    9 // uint8_t/RsServerStateType/RS_PROTOCOL_AUTH_TOKEN1...
#define RS_PROTOCOL_INDEX_DATA6   10 // uint8_t/RsServerStateType/RS_PROTOCOL_AUTH_TOKEN1...
#define RS_PROTOCOL_INDEX_DATA7   11 // uint8_t/RsServerStateType/RS_PROTOCOL_AUTH_TOKEN1...
#define RS_PROTOCOL_INDEX_DATA8   12 // uint8_t/RsServerStateType/RS_PROTOCOL_AUTH_TOKEN1...
#define RS_PROTOCOL_INDEX_CRC     13 // uint8_t/^ all from model to last data
#define RS_PROTOCOL_INDEX_TAIL    14 // uint8_t/RS_PROTOCOL_TAIL

// token “20220718”
#define RS_PROTOCOL_AUTH_TOKEN1 0x30 // 0
#define RS_PROTOCOL_AUTH_TOKEN2 0x37 // 7
#define RS_PROTOCOL_AUTH_TOKEN3 0x31 // 1
#define RS_PROTOCOL_AUTH_TOKEN4 0x38 // 8

typedef enum {
  RS_SERVER_MODEL_UNKNOWN     = RS_PROTOCOL_UNKNOWN,
  RS_SERVER_MODEL_DOOR_SENSOR = 68, // D
} RsServerModelType;

typedef enum {
  RS_SERVER_STATE_UNKNOWN   = RS_PROTOCOL_UNKNOWN,
  RS_SERVER_STATE_BATTERY   = 66, // B
  RS_SERVER_STATE_OTA_ON    = 65, // A
  RS_SERVER_STATE_INPUT_ON  = 73, // I
  RS_SERVER_STATE_OUTPUT_ON = 79, // O
} RsServerStateType;

typedef enum {
  RS_SERVER_COMMAND_UNKNOWN       = RS_PROTOCOL_UNKNOWN,
  RS_SERVER_COMMAND_AUTHENTICATE  = 65, // A
  RS_SERVER_COMMAND_HEARTBEAT     = 72, // H
  RS_SERVER_COMMAND_RESET         = 82, // R
  RS_SERVER_COMMAND_GET_STATES    = 71, // G
  RS_SERVER_COMMAND_SET_STATES    = 83, // S
  RS_SERVER_COMMAND_NOTIFY_STATES = 78, // N
} RsServerCommandType;

uint8_t rsGenerateCRC(uint8_t* payload);
bool rsAuthenticate(uint8_t* payload);
void rsFillMessage(uint8_t* payload, uint8_t model, uint8_t command, uint8_t data1, uint8_t data2, uint8_t data3, uint8_t data4, uint8_t data5, uint8_t data6, uint8_t data7, uint8_t data8);
bool rsValidateMessage(uint8_t* payload, size_t length);

#endif
