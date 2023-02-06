/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_app_led_test.c
 * @brief       led demo
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-04-09
 * @version     1.0
 ******************************************************************************/

#include "main.h"

#ifdef EC_APP_LED_TEST

#include "ec_core.h"

#define EC_APP_UART0_TX_BUF_SIZE 1024                 //串口0发送缓冲区大小，可以根据需要调整
#define EC_APP_UART0_RX_BUF_SIZE 1024                 //串口0接收缓冲区大小，可以根据需要调整
uint8_t uart0_tx_buf[EC_APP_UART0_TX_BUF_SIZE] = {0}; //串口0发送缓冲区
uint8_t uart0_rx_buf[EC_APP_UART0_RX_BUF_SIZE] = {0}; //串口0接收缓冲区

void uart0_rx(uint8_t *buf, uint16_t len) //串口0接收数据中断
{
    if (buf[0] == '0')
        ec_core_led_set_mode(EC_CORE_LED_MODE_OFF); // led off
    if (buf[0] == '1')
        ec_core_led_set_mode(EC_CORE_LED_MODE_2S); // 2秒闪烁
    if (buf[0] == '2')
        ec_core_led_set_mode(EC_CORE_LED_MODE_1S); // 1秒闪烁
    if (buf[0] == '3')
        ec_core_led_set_mode(EC_CORE_LED_MODE_200MS); // 200ms闪烁
    if (buf[0] == '4')
        ec_core_led_set_mode(EC_CORE_LED_MODE_ON); // led on
}

void uart0_init(void) //串口0初始化 波特率精度受时钟频率影响
{
    ec_core_uart_init(EC_CORE_UART0, 115200, EC_CORE_UART_PARITY_NONE,
                      EC_CORE_GPIO_P4, EC_CORE_GPIO_P5,
                      uart0_tx_buf, EC_APP_UART0_TX_BUF_SIZE, uart0_rx_buf, EC_APP_UART0_RX_BUF_SIZE,
                      uart0_rx);
}

int main(void)
{
    ec_core_sys_clk_set(EC_CORE_SYS_CLK_48M); //系统时钟初始化

    ec_core_init(); //蓝牙内核初始化

    uart0_init();                                //串口0初始化 波特率精度受时钟频率影响
    ec_core_uart0_printf("ec_app_led_test\r\n"); //串口0 printf打印

    ec_core_led_init(EC_CORE_GPIO_P0, EC_CORE_SW_TIMER6); // LED初始化 占用一个gpio和一个软件定时器

    ec_core_sleep_disable(); //禁止睡眠，串口可以接收数据
    ec_core_main_loop_run(); //系统主循环
}

#endif
