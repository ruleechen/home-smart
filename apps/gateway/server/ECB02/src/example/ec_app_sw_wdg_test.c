/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_app_sw_wdg_test.c
 * @brief       software watchdog test
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-04-09
 * @version     1.0
 ******************************************************************************/

#include "main.h"

#ifdef EC_APP_SW_WDG_TEST

#include "ec_core.h"

#define EC_APP_UART0_TX_BUF_SIZE 1024                 //串口0发送缓冲区大小，可以根据需要调整
#define EC_APP_UART0_RX_BUF_SIZE 1024                 //串口0接收缓冲区大小，可以根据需要调整
uint8_t uart0_tx_buf[EC_APP_UART0_TX_BUF_SIZE] = {0}; //串口0发送缓冲区
uint8_t uart0_rx_buf[EC_APP_UART0_RX_BUF_SIZE] = {0}; //串口0接收缓冲区

void uart0_rx(uint8_t *buf, uint16_t len) //串口0接收数据中断
{
}

void uart0_init(void) //串口0初始化 波特率精度受时钟频率影响
{
    ec_core_uart_init(EC_CORE_UART0, 115200, EC_CORE_UART_PARITY_NONE,
                      EC_CORE_GPIO_P4, EC_CORE_GPIO_P5,
                      uart0_tx_buf, EC_APP_UART0_TX_BUF_SIZE, uart0_rx_buf, EC_APP_UART0_RX_BUF_SIZE,
                      uart0_rx);
}

static void ec_app_ble_peripheral_receive_event(uint8_t *data, uint8_t len) //蓝牙接收数据回调
{
    // ec_core_uart0_printf("ble peripheral receive len=%d\r\n", len);
    ec_core_uart_send(EC_CORE_UART0, data, len); //蓝牙数据转发到串口
    // ec_core_uart0_printf("\r\n");

    ec_core_sw_watchdog_feed(); //软件看门狗喂狗
}

int main(void)
{
    ec_core_sys_clk_set(EC_CORE_SYS_CLK_48M); //系统时钟初始化

    ec_core_ble_peripheral_set_receive_cb(ec_app_ble_peripheral_receive_event); //注册蓝牙数据接收的回调

    ec_core_init(); //蓝牙内核初始化

    uart0_init(); //串口0初始化 波特率精度受时钟频率影响

    ec_core_uart0_printf("ec_app_sw_wdg_test\r\n"); //串口0 printf打印

    ec_core_sw_watchdog_init(EC_CORE_SW_TIMER6, 2, 3); //初始化软件看门狗，广播超时时间2分钟，蓝牙连接超时时间3分钟

    ec_core_sleep_disable(); //禁止睡眠，串口可以接收数据
    ec_core_main_loop_run(); //系统主循环
}

#endif
