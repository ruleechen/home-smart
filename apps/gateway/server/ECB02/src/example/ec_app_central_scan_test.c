/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_app_central_scan_test.c
 * @brief       central scan demo
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-02-13
 * @version     1.0
 ******************************************************************************/

#include "main.h"

#ifdef EC_APP_CENTRAL_SCAN_TEST

#include "ec_core.h"

#define EC_APP_UART0_TX_BUF_SIZE 1024                 //串口0发送缓冲区大小，可以根据需要调整
#define EC_APP_UART0_RX_BUF_SIZE 1024                 //串口0接收缓冲区大小，可以根据需要调整
uint8_t uart0_tx_buf[EC_APP_UART0_TX_BUF_SIZE] = {0}; //串口0发送缓冲区
uint8_t uart0_rx_buf[EC_APP_UART0_RX_BUF_SIZE] = {0}; //串口0接收缓冲区

void scan_cb(uint8_t count) //主机蓝牙扫描回调
{
    ec_core_uart0_printf("scan_cb count = %d\r\n", count); //扫描到的从机数量
    for (uint8_t i = 0; i < count; i++)
    {
        ec_core_ble_central_scan_device device;
        ec_core_ble_central_get_device(i, &device); //获取从机参数
        ec_core_uart0_printf("%d,%s,%02X%02X%02X%02X%02X%02X,%d\r\n",
                             i, device.name,                              //序号、蓝牙名字
                             device.mac[0], device.mac[1], device.mac[2], // MAC地址
                             device.mac[3], device.mac[4], device.mac[5], // MAC地址
                             device.rssi);                                //信号强度
    }
}

void ble_central_connect_ok_cb(void) //蓝牙连接成功回调
{
    ec_core_uart0_printf("ble_central_connect_ok\r\n");
}
void ble_central_connect_fail_cb(void) //蓝牙连接失败回调
{
    ec_core_uart0_printf("ble_central_connect_fail\r\n");
    ec_core_sys_soft_reset(); //系统复位
}
void ble_central_disconnect_cb(void) //蓝牙断开回调
{
    ec_core_uart0_printf("ble_central_disconnect\r\n");
    ec_core_sys_soft_reset(); //系统复位
}

void ble_central_receive_cb(uint8_t *data, uint8_t len) //蓝牙接收数据
{
    ec_core_uart0_printf("ble central receive len=%d\r\n", len);
    ec_core_uart_send(EC_CORE_UART0, data, len);
    ec_core_uart0_printf("\r\n");
}

void uart0_rx(uint8_t *buf, uint16_t len) //串口0接收数据中断
{
    if (strcmp((const char *)buf, "SCAN") == 0)
    {
        ec_core_ble_central_scan(); //开始扫描
    }
    else if (buf[0] == 'A')
    {
        ec_core_ble_central_connect(buf[1] - '0'); //连接设备列表里的某一台设备
    }
    else if (strcmp((const char *)buf, "DISC") == 0)
    {
        ec_core_ble_disconnect(); //主动断开蓝牙连接
    }
    else
    {
        ec_core_ble_send(buf, len); //串口数据转发到蓝牙
    }
}

void uart0_init(void) //串口0初始化 波特率精度受时钟频率影响
{
    ec_core_uart_init(EC_CORE_UART0, 115200, EC_CORE_UART_PARITY_NONE,
                      EC_CORE_GPIO_P4, EC_CORE_GPIO_P5,
                      uart0_tx_buf, EC_APP_UART0_TX_BUF_SIZE, uart0_rx_buf, EC_APP_UART0_RX_BUF_SIZE,
                      uart0_rx);
}

void sw_timer1_int(void)
{
    ec_core_sw_timer_stop(EC_CORE_SW_TIMER1);
    ec_core_ble_central_scan(); //开始扫描
}

int main(void)
{
    //主机模式，手动扫描，手动连接

    ec_core_sys_clk_set(EC_CORE_SYS_CLK_48M); //系统时钟初始化

    ec_core_set_role(EC_CORE_BLE_ROLE_CENTRAL);                                                            //主机模式
    ec_core_ble_central_set_duration(1000);                                                                //扫描持续时间1秒
    ec_core_ble_central_set_scan_cb(scan_cb);                                                              //注册蓝牙扫描回调
    ec_core_ble_central_set_event_cb(EC_CORE_BLE_CENTRAL_EVENT_CONNECT_OK, ble_central_connect_ok_cb);     //注册蓝牙连接成功回调
    ec_core_ble_central_set_event_cb(EC_CORE_BLE_CENTRAL_EVENT_CONNECT_FAIL, ble_central_connect_fail_cb); //注册蓝牙连接失败回调
    ec_core_ble_central_set_event_cb(EC_CORE_BLE_CENTRAL_EVENT_DISCONNECT, ble_central_disconnect_cb);     //注册蓝牙断开回调
    ec_core_ble_central_set_receive_cb(ble_central_receive_cb);                                            //注册蓝牙接收数据回调

    ec_core_init(); //蓝牙内核初始化

    uart0_init();                                         //串口0初始化 波特率精度受时钟频率影响
    ec_core_uart0_printf("ec_app_central_scan_test\r\n"); //串口0 printf打印

    ec_core_sw_timer_start(EC_CORE_SW_TIMER1, 1000, sw_timer1_int); //启动软件定时器1,周期1秒

    ec_core_sleep_disable(); //禁止睡眠，串口可以接收数据
    ec_core_main_loop_run(); //系统主循环
}

#endif
