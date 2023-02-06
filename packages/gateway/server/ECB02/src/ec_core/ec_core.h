/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_core.h
 * @brief       eciot core api
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-02-13
 * @version     1.0
 ******************************************************************************/

#ifndef __EC_CORE_H__
#define __EC_CORE_H__

#include "stdint.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

#include "ec_core_clk.h"
#include "ec_core_delay.h"
#include "ec_core_gpio.h"
#include "ec_core_pwm.h"
#include "ec_core_adc.h"
#include "ec_core_uart.h"
#include "ec_core_printf.h"
#include "ec_core_led.h"
#include "ec_core_key.h"
#include "ec_core_timer.h"
#include "ec_core_sleep.h"
#include "ec_core_wdg.h"
#include "ec_core_flash.h"
#include "ec_core_i2c.h"
#include "ec_core_spi_ssd1306_oled.h"
#include "ec_core_ble_peripheral.h"
#include "ec_core_ble_central.h"
#include "ec_core_ble.h"

extern void ec_core_init(void);                                 //蓝牙内核初始化
extern void ec_core_main_loop_run(void);                        //系统主循环
extern void ec_core_sys_soft_reset(void);                       //软件复位
extern void ec_core_rand(uint8_t *rand_data, uint8_t data_len); //随机数生成器
extern void ec_core_power_off(ec_core_gpio_pin_e pin);          //进入关机模式
extern void ec_core_ver(uint8_t *ver);                          //查询软件版本

#endif
