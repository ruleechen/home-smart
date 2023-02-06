/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_app_ble_peripheral_test.c
 * @brief       ble peripheral demo
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-02-13
 * @version     1.0
 ******************************************************************************/

#include "main.h"

#ifdef EC_APP_BLE_PERIPHERAL_TEST

#include "ec_core.h"

#define EC_APP_UART0_TX_BUF_SIZE 1024                 //串口0发送缓冲区大小，可以根据需要调整
#define EC_APP_UART0_RX_BUF_SIZE 1024                 //串口0接收缓冲区大小，可以根据需要调整
uint8_t uart0_tx_buf[EC_APP_UART0_TX_BUF_SIZE] = {0}; //串口0发送缓冲区
uint8_t uart0_rx_buf[EC_APP_UART0_RX_BUF_SIZE] = {0}; //串口0接收缓冲区

void uart0_rx(uint8_t *buf, uint16_t len) //串口0接收数据中断
{
    ec_core_ble_send(buf, len); //串口数据转发到蓝牙

    if (strcmp((const char *)buf, "DISC") == 0)
        ec_core_ble_disconnect(); //主动断开蓝牙连接
}

void uart0_init(void) //串口0初始化 波特率精度受时钟频率影响
{
    ec_core_uart_init(EC_CORE_UART0, 115200, EC_CORE_UART_PARITY_NONE,
                      EC_CORE_GPIO_P4, EC_CORE_GPIO_P5,
                      uart0_tx_buf, EC_APP_UART0_TX_BUF_SIZE, uart0_rx_buf, EC_APP_UART0_RX_BUF_SIZE,
                      uart0_rx);
}

static void ec_app_ble_peripheral_connect_event(void) //蓝牙连接成功回调
{
    ec_core_uart0_printf("ble peripheral connect\r\n");
}
static void ec_app_ble_peripheral_disconnect_event(void) //蓝牙断开连接回调
{
    ec_core_uart0_printf("ble peripheral disconnect\r\n");
}
static void ec_app_ble_peripheral_notify_enable_event(void) //订阅打开回调
{
    ec_core_uart0_printf("ble peripheral notify enable\r\n");
}
static void ec_app_ble_peripheral_notify_disable_event(void) //订阅关闭回调
{
    ec_core_uart0_printf("ble peripheral notify disable\r\n");
}
static void ec_app_ble_peripheral_receive_event(uint8_t *data, uint8_t len) //蓝牙接收数据回调
{
    ec_core_uart0_printf("ble peripheral receive len=%d\r\n", len);
    ec_core_uart_send(EC_CORE_UART0, data, len);
    ec_core_uart0_printf("\r\n");
}

int main(void)
{
    ec_core_sys_clk_set(EC_CORE_SYS_CLK_48M); //系统时钟初始化

    ec_core_ble_peripheral_set_event_cb(EC_CORE_BLE_PERIPHERAL_EVENT_CONNECT, ec_app_ble_peripheral_connect_event);               //注册蓝牙连接的回调
    ec_core_ble_peripheral_set_event_cb(EC_CORE_BLE_PERIPHERAL_EVENT_DISCONNECT, ec_app_ble_peripheral_disconnect_event);         //注册蓝牙断开的回调
    ec_core_ble_peripheral_set_event_cb(EC_CORE_BLE_PERIPHERAL_EVENT_NOTIFY_ENABLE, ec_app_ble_peripheral_notify_enable_event);   //注册蓝牙订阅打开的回调
    ec_core_ble_peripheral_set_event_cb(EC_CORE_BLE_PERIPHERAL_EVENT_NOTIFY_DISABLE, ec_app_ble_peripheral_notify_disable_event); //注册蓝牙订阅关闭的回调
    ec_core_ble_peripheral_set_receive_cb(ec_app_ble_peripheral_receive_event);                                                   //注册蓝牙数据接收的回调

    ec_core_init(); //蓝牙内核初始化

    uart0_init(); //串口0初始化 波特率精度受时钟频率影响

    ec_core_uart0_printf("ec_app_ble_peripheral_test\r\n"); //串口0 printf打印

    ec_core_sleep_disable(); //禁止睡眠，串口可以接收数据
    ec_core_main_loop_run(); //系统主循环
}

#endif
