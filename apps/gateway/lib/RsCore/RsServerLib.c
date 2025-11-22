#include "RsServerLib.h"

RsServerModelType _model;
bool rsAuthenticated = false;
static RsServerEmitHandler _emitHandler = NULL;
static RsServerCommandHandler _commandHandler = NULL;

void rsEmitStates(uint8_t command, uint8_t data1, uint8_t data2, uint8_t data3, uint8_t data4, uint8_t data5, uint8_t data6, uint8_t data7, uint8_t data8) {
  uint8_t payload[RS_PROTOCOL_BYTES] = {0};
  rsFillMessage(payload, _model, command, data1, data2, data3, data4, data5, data6, data7, data8);
  _emitHandler(payload, RS_PROTOCOL_BYTES);
}

void rsHandleMessage(uint8_t* payload, size_t length) {
  bool valid = rsValidateMessage(payload, length);
  if (valid == false) {
    _commandHandler(NULL, 0);
    return;
  }
  if (
    rsAuthenticated == true &&
    payload[RS_PROTOCOL_INDEX_MODEL] != _model
  ) {
    _commandHandler(NULL, 0);
    return;
  }
  if (rsAuthenticated == false) {
    rsAuthenticated = rsAuthenticate(payload);
  }
  _commandHandler(payload, length);
}

void rsServerInit(
  RsServerModelType model,
  RsServerEmitHandler emitHandler,
  RsServerCommandHandler commandHandler
) {
  _model = model;
  _emitHandler = emitHandler;
  _commandHandler = commandHandler;
}
