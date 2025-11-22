#ifndef __EC_APP_FLASH_H__
#define __EC_APP_FLASH_H__

#include "stdint.h"
#include "ec_core.h"

extern uint8_t rsOtaEnabled;

extern void ec_app_flash_sys_param_write(void); //系统参数保存到flash
extern void ec_app_flash_sys_param_read(void);  //从flash中读取系统参数

#endif
