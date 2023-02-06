/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_core_wdg.c
 * @brief       wdg control
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-04-09
 * @version     1.0
 ******************************************************************************/

#include "ec_core.h"

static ec_core_sw_timer_e ec_core_sw_wdg_timer;
static uint16_t ec_core_sw_wdg_ble_adv_time = 0;
static uint16_t ec_core_sw_wdg_ble_connect_time = 0;
static uint16_t ec_core_sw_wdg_ble_adv_timeout = 2;     // 2分钟
static uint16_t ec_core_sw_wdg_ble_connect_timeout = 3; // 3分钟

void ec_core_sw_watchdog_feed(void) //软件看门狗喂狗
{
    ec_core_sw_wdg_ble_adv_time = 0;
    ec_core_sw_wdg_ble_connect_time = 0;
}

static void ec_core_sw_watchdog_10s_task(void)
{
    if (ec_core_ble_get_link_state() == EC_CORE_BLE_LINK_STATE_DISCONNECT)
    {
        if (++ec_core_sw_wdg_ble_adv_time > (ec_core_sw_wdg_ble_adv_timeout * 6))
        {
            ec_core_sw_wdg_ble_adv_time = 0;
            ec_core_sys_soft_reset();
        }
    }
    else
    {
        ec_core_sw_wdg_ble_adv_time = 0;
    }

    if (ec_core_ble_get_link_state() == EC_CORE_BLE_LINK_STATE_CONNECT)
    {
        if (++ec_core_sw_wdg_ble_connect_time > (ec_core_sw_wdg_ble_connect_timeout * 6))
        {
            ec_core_sw_wdg_ble_connect_time = 0;
            ec_core_sys_soft_reset();
        }
    }
    else
    {
        ec_core_sw_wdg_ble_connect_time = 0;
    }
}

// 软件看门狗初始化
// ec_core_sw_timer_e  软件定时器
// ble_adv_timeout 蓝牙处于广播状态超时时间，单位是分钟
// ble_connect_timeout 蓝牙处于连接状态超时时间，单位是分钟
// 芯片长时间处于蓝牙广播状态，会定时复位
// 芯片长时间处于蓝牙连接状态，没有数据通讯，会超时复位，避免被长时间占用
// 提升芯片在复杂环境下从异常状态恢复的能力，提高系统的鲁棒性
void ec_core_sw_watchdog_init(ec_core_sw_timer_e timer, uint16_t ble_adv_timeout, uint16_t ble_connect_timeout)
{
    ec_core_sw_wdg_timer = timer;
    ec_core_sw_wdg_ble_adv_timeout = ble_adv_timeout;
    ec_core_sw_wdg_ble_connect_timeout = ble_connect_timeout;
    ec_core_sw_timer_start(ec_core_sw_wdg_timer, 10000, ec_core_sw_watchdog_10s_task);
}
