/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_core_ble_central.h
 * @brief       ble central control
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-02-13
 * @version     1.0
 ******************************************************************************/

#ifndef __EC_CORE_BLE_CENTRAL_H__
#define __EC_CORE_BLE_CENTRAL_H__

#include "stdint.h"

//主机扫描到的设备
typedef struct
{
    uint8_t name[21]; //蓝牙名称
    uint8_t name_len; //蓝牙名称长度
    uint8_t mac[6];   // MAC地址
    int16_t rssi;     //信号强度
} ec_core_ble_central_scan_device;

//蓝牙主机回调类型
typedef enum
{
    EC_CORE_BLE_CENTRAL_EVENT_CONNECT_OK,
    EC_CORE_BLE_CENTRAL_EVENT_CONNECT_FAIL,
    EC_CORE_BLE_CENTRAL_EVENT_DISCONNECT,
} ec_core_ble_central_event_e;

//扫描回调 count：搜索到的从机数量
typedef void (*ec_core_ble_central_scan_event_t)(uint8_t count);

typedef void (*ec_core_ble_central_event_t)(void);
typedef void (*ec_core_ble_central_receive_t)(uint8_t *data, uint8_t len); //接收数据回调

//设置扫描持续时间，单位毫秒
extern void ec_core_ble_central_set_duration(uint16_t ms);
//注册扫描回调
extern void ec_core_ble_central_set_scan_cb(ec_core_ble_central_scan_event_t cb);
//开始扫描
extern void ec_core_ble_central_scan(void);
//获取扫描到的设备
extern void ec_core_ble_central_get_device(uint8_t index, ec_core_ble_central_scan_device *device);
//开始连接从机
extern void ec_core_ble_central_connect(uint8_t i);
//设置主机事件回调
extern void ec_core_ble_central_set_event_cb(ec_core_ble_central_event_e event_type, ec_core_ble_central_event_t cb);
//设置蓝牙数据接收回调
extern void ec_core_ble_central_set_receive_cb(ec_core_ble_central_receive_t cb);

//主机绑定从机MAC地址，自动搜索，自动连接，蓝牙断开自动重连
extern void ec_core_ble_central_bond_mac(uint8_t *mac);

#endif
