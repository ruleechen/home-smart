/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_core_ble_peripheral.h
 * @brief       ble peripheral control
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-02-13
 * @version     1.0
 ******************************************************************************/

#ifndef __EC_CORE_BLE_PERIPHERAL_H__
#define __EC_CORE_BLE_PERIPHERAL_H__

#include "stdint.h"

//蓝牙从机事件
typedef enum
{
    EC_CORE_BLE_PERIPHERAL_EVENT_CONNECT,
    EC_CORE_BLE_PERIPHERAL_EVENT_DISCONNECT,
    EC_CORE_BLE_PERIPHERAL_EVENT_NOTIFY_ENABLE,
    EC_CORE_BLE_PERIPHERAL_EVENT_NOTIFY_DISABLE,
} ec_core_ble_peripheral_event_e;

//蓝牙订阅状态
typedef enum
{
    EC_CORE_BLE_PERIPHERAL_NOTIFY_STATE_DISABLE,
    EC_CORE_BLE_PERIPHERAL_NOTIFY_STATE_ENABLE,
} ec_core_ble_peripheral_notify_state_e;

typedef void (*ec_core_ble_peripheral_event_t)(void);
typedef void (*ec_core_ble_peripheral_receive_t)(uint8_t *data, uint8_t len);

// 1:open 0:close  default is open  打开或关闭OTA功能，默认打开
extern void ec_core_ble_peripheral_set_ota_en(uint8_t p);
//设置蓝牙名称 最长20字节
extern void ec_core_ble_peripheral_set_name(uint8_t *name, uint8_t len);
//设置广播间隙 单位是625us，数值范围32~16384，时间范围20ms~10.24s  默认是320(200MS)
extern void ec_core_ble_peripheral_set_adv_int(uint16_t n);
//设置厂商自定义数据 最长22字节
extern void ec_core_ble_peripheral_set_manufacturer_data(uint8_t *data, uint8_t len);
//清除厂商自定义数据
extern void ec_core_ble_peripheral_clear_manufacturer_data(void);
//设置自定义蓝牙广播包，最长31字节
extern void ec_core_ble_peripheral_set_custom_adv_data(uint8_t *data, uint8_t len);
//清除自定义蓝牙广播包
extern void ec_core_ble_peripheral_clear_custom_adv_data(void);
//设置自定义扫描应答包
extern void ec_core_ble_peripheral_set_custom_scan_rsp(uint8_t *data, uint8_t len);
//清除自定义扫描应答包
extern void ec_core_ble_peripheral_clear_custom_scan_rsp(void);
//重新广播，修改蓝牙广播参数后，需要重新广播，才能生效。
extern void ec_core_ble_peripheral_reset_adv(void);

//设置蓝牙连接参数
// con_int_min   数值范围 6-3200  单位 1.25ms  默认为 7.5 毫秒
// con_int_max   数值范围 6-3200  单位 1.25ms  默认为 15 毫秒
// latency   数值范围 0-499   默认为 0
// con_timeout   数值范围 1-3200   单位 10ms   默认为 3 秒
void ec_core_ble_peripheral_set_connect_param(uint16_t con_int_min, uint16_t con_int_max, uint16_t latency, uint16_t con_timeout);

//注册蓝牙从机事件回调
extern void ec_core_ble_peripheral_set_event_cb(ec_core_ble_peripheral_event_e event_type, ec_core_ble_peripheral_event_t cb);
//注册蓝牙接收数据事件回调
extern void ec_core_ble_peripheral_set_receive_cb(ec_core_ble_peripheral_receive_t cb);

//查询蓝牙订阅状态
extern ec_core_ble_peripheral_notify_state_e ec_core_ble_peripheral_get_notify_state(void);

#endif
