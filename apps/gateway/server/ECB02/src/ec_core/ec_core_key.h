/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_core_key.h
 * @brief       key control
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-04-09
 * @version     1.0
 ******************************************************************************/

#ifndef __EC_CORE_KEY_H__
#define __EC_CORE_KEY_H__

#include "ec_core.h"
#include "ec_core_timer.h"

typedef void (*ec_core_key_event_t)(void); // key事件

//引脚  定时器  长按时间(单位100ms)  按键按下事件  按键长按事件  按键抬起事件
extern void ec_core_key_init(ec_core_gpio_pin_e pin, ec_core_sw_timer_e timer, uint8_t long_press_time_unit_100ms,
                             ec_core_key_event_t press_event, ec_core_key_event_t long_press_event, ec_core_key_event_t up_event);

#endif
