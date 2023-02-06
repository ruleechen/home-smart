/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_app_printf_test.c
 * @brief       printf demo
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-04-08
 * @version     1.0
 ******************************************************************************/

#include "main.h"

#ifdef EC_APP_PRINTF_TEST

#include "ec_core.h"

#define EC_APP_UART0_TX_BUF_SIZE 1024                 //串口0发送缓冲区大小，可以根据需要调整
#define EC_APP_UART0_RX_BUF_SIZE 1024                 //串口0接收缓冲区大小，可以根据需要调整
#define EC_APP_UART1_TX_BUF_SIZE 1024                 //串口1发送缓冲区大小，可以根据需要调整
#define EC_APP_UART1_RX_BUF_SIZE 1024                 //串口1接收缓冲区大小，可以根据需要调整
uint8_t uart0_tx_buf[EC_APP_UART0_TX_BUF_SIZE] = {0}; //串口0发送缓冲区
uint8_t uart0_rx_buf[EC_APP_UART0_RX_BUF_SIZE] = {0}; //串口0接收缓冲区
uint8_t uart1_tx_buf[EC_APP_UART1_TX_BUF_SIZE] = {0}; //串口1发送缓冲区
uint8_t uart1_rx_buf[EC_APP_UART1_RX_BUF_SIZE] = {0}; //串口1接收缓冲区

void uart0_rx(uint8_t *buf, uint16_t len) //串口0接收数据中断
{
    ec_core_uart0_printf("uart0_rx len=%d\r\n", len); // uart0 printf 打印
    ec_core_uart_send(EC_CORE_UART0, buf, len);       // ECHO 回显
    ec_core_ble_send(buf, len);                       //串口数据转发到蓝牙
}

void uart0_init(void) //串口0初始化 波特率精度受时钟频率影响
{
    ec_core_uart_init(EC_CORE_UART0, 115200, EC_CORE_UART_PARITY_NONE,
                      EC_CORE_GPIO_P4, EC_CORE_GPIO_P5,
                      uart0_tx_buf, EC_APP_UART0_TX_BUF_SIZE, uart0_rx_buf, EC_APP_UART0_RX_BUF_SIZE,
                      uart0_rx);
}

void uart1_rx(uint8_t *buf, uint16_t len) //串口1接收数据中断
{
    ec_core_uart1_printf("uart1_rx len=%d\r\n", len); // uart1 printf 打印
    ec_core_uart_send(EC_CORE_UART1, buf, len);       // ECHO 回显
}

void uart1_init(void) //串口1初始化 波特率精度受时钟频率影响
{
    ec_core_uart_init(EC_CORE_UART1, 115200, EC_CORE_UART_PARITY_NONE,
                      EC_CORE_GPIO_P1, EC_CORE_GPIO_P2,
                      uart1_tx_buf, EC_APP_UART1_TX_BUF_SIZE, uart1_rx_buf, EC_APP_UART1_RX_BUF_SIZE,
                      uart1_rx);
}

void sw_timer1_int(void) //软件定时器1回调
{
    uint8_t flag = 88;
    ec_core_sw_timer_stop(EC_CORE_SW_TIMER1);            // 停止定时器  只执行一次
    ec_core_ble_printf("ble receive data %d\r\n", flag); // 蓝牙 printf 打印
}

void ec_app_ble_peripheral_receive_event(uint8_t *data, uint8_t len) //蓝牙接收数据回调
{
    ec_core_uart_send(EC_CORE_UART0, data, len); //串口透传

    ec_core_sw_timer_stop(EC_CORE_SW_TIMER1);
    ec_core_sw_timer_start(EC_CORE_SW_TIMER1, 100, sw_timer1_int); //放到定时器中执行
}

void sw_timer2_int(void) //软件定时器2回调
{
    static uint8_t t = 0;
    ec_core_ble_printf("hi %d\r\n", t); //蓝牙 printf 打印
    t++;
}

int main(void)
{
    ec_core_sys_clk_set(EC_CORE_SYS_CLK_48M); //系统时钟初始化

    ec_core_ble_peripheral_set_receive_cb(ec_app_ble_peripheral_receive_event); //注册蓝牙数据接收的回调

    ec_core_init(); //蓝牙内核初始化

    uart0_init(); //串口0初始化 波特率精度受时钟频率影响
    uart1_init(); //串口1初始化 波特率精度受时钟频率影响

    ec_core_uart0_printf("ec_app_printf_test\r\n"); //串口0 printf打印
    ec_core_uart1_printf("ec_app_printf_test\r\n"); //串口1 printf打印

    ec_core_sw_timer_start(EC_CORE_SW_TIMER2, 1000, sw_timer2_int); //软件定时器2  1秒执行一次

    ec_core_sleep_disable(); //禁止睡眠，串口可以接收数据
    ec_core_main_loop_run(); //系统主循环
}

#endif
