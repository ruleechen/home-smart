/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_core_wdg.h
 * @brief       wdg control
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-04-09
 * @version     1.0
 ******************************************************************************/

#ifndef __EC_CORE_WDG_H__
#define __EC_CORE_WDG_H__

#include "ec_core.h"

// ec_core_main_loop_run函数中有在喂狗，如果使用自己的while(1),需要调用喂狗函数。
extern void ec_core_hw_watchdog_feed(void); //硬件看门狗喂狗

extern void ec_core_sw_watchdog_feed(void); //软件看门狗喂狗

// 软件看门狗初始化
// ec_core_sw_timer_e  软件定时器
// ble_adv_timeout 蓝牙处于广播状态超时时间，单位是分钟
// ble_connect_timeout 蓝牙处于连接状态超时时间，单位是分钟
// 芯片长时间处于蓝牙广播状态，会定时复位
// 芯片长时间处于蓝牙连接状态，没有数据通讯，会超时复位，避免被长时间占用
// 提升芯片在复杂环境下从异常状态恢复的能力，提高系统的鲁棒性
extern void ec_core_sw_watchdog_init(ec_core_sw_timer_e timer, uint16_t ble_adv_timeout, uint16_t ble_connect_timeout);

#endif
