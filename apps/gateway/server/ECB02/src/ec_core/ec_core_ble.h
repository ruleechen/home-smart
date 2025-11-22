/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_core_ble.h
 * @brief       ble control
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-02-13
 * @version     1.0
 ******************************************************************************/

#ifndef __EC_CORE_BLE_H__
#define __EC_CORE_BLE_H__

#include "stdint.h"

//主机或从机 默认从机
typedef enum
{
    EC_CORE_BLE_ROLE_PERIPHERAL,
    EC_CORE_BLE_ROLE_CENTRAL,
} ec_core_ble_role_e;

//蓝牙连接状态
typedef enum
{
    EC_CORE_BLE_LINK_STATE_DISCONNECT,
    EC_CORE_BLE_LINK_STATE_CONNECT,
} ec_core_ble_link_state_e;

//蓝牙发射功率
typedef enum
{
    EC_CORE_BLE_POWER_N20DBM, // -20dbm  最小功率
    EC_CORE_BLE_POWER_N15DBM, // -15dbm
    EC_CORE_BLE_POWER_N10DBM, // -10dbm
    EC_CORE_BLE_POWER_N6DBM,  // -6dbm
    EC_CORE_BLE_POWER_N5DBM,  // -5dbm
    EC_CORE_BLE_POWER_N2DBM,  // -2dbm
    EC_CORE_BLE_POWER_0DBM,   // 0dbm
    EC_CORE_BLE_POWER_3DBM,   // 3dbm
    EC_CORE_BLE_POWER_4DBM,   // 4dbm
    EC_CORE_BLE_POWER_5DBM,   // 5dbm
    EC_CORE_BLE_POWER_7DBM,   // 7dbm
    EC_CORE_BLE_POWER_10DBM,  // 10dbm  最大功率
} ec_core_ble_power_e;

//配置主机或从机 默认从机
extern void ec_core_set_role(ec_core_ble_role_e role);

extern void ec_core_ble_get_mac(uint8_t *mac); //获取MAC地址
extern void ec_core_ble_set_mac(uint8_t *mac); //设置MAC地址

extern void ec_core_ble_set_suuid(char *uuid); //配置服务UUID
extern void ec_core_ble_set_ruuid(char *uuid); //配置特性UUID
extern void ec_core_ble_set_wuuid(char *uuid); //配置特性UUID

extern void ec_core_ble_set_power(ec_core_ble_power_e p); //配置蓝牙发射功率

extern void ec_core_ble_send(uint8_t *data, uint16_t len); //蓝牙发送数据

extern uint16_t ec_core_ble_get_mtu(void);                        //获取MTU值
extern ec_core_ble_link_state_e ec_core_ble_get_link_state(void); //获取蓝牙连接状态

extern void ec_core_ble_disconnect(void); //主动断开蓝牙连接

#endif
