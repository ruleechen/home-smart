#ifndef RsServerLib_h
#define RsServerLib_h

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "RsProtocol.h"

extern bool rsAuthenticated;
typedef void (*RsServerEmitHandler)(uint8_t* payload, size_t length);
typedef void (*RsServerCommandHandler)(uint8_t* payload, size_t length);

extern void rsEmitStates(uint8_t command, uint8_t data1, uint8_t data2, uint8_t data3, uint8_t data4, uint8_t data5, uint8_t data6, uint8_t data7, uint8_t data8);
extern void rsHandleMessage(uint8_t* payload, size_t length);
extern void rsServerInit(
  RsServerModelType model,
  RsServerEmitHandler emitHandler,
  RsServerCommandHandler commandHandler
);

#endif
