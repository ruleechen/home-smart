/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_core_led.h
 * @brief       led control
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-04-09
 * @version     1.0
 ******************************************************************************/

#ifndef __EC_CORE_LED_H__
#define __EC_CORE_LED_H__

#include "ec_core.h"
#include "ec_core_timer.h"

typedef enum
{
    EC_CORE_LED_MODE_OFF,   // led off
    EC_CORE_LED_MODE_2S,    // 2秒闪烁
    EC_CORE_LED_MODE_1S,    // 1秒闪烁
    EC_CORE_LED_MODE_200MS, // 200毫秒闪烁
    EC_CORE_LED_MODE_ON,    // led on
} ec_core_led_mode_e;

extern void ec_core_led_init(ec_core_gpio_pin_e pin, ec_core_sw_timer_e timer); //初始化led 需要占用一个引脚和一个软件定时器
extern void ec_core_led_set_mode(ec_core_led_mode_e mode);                      //设置led的工作模式

#endif
