/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_app_sleep_test.c
 * @brief       sleep demo
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-02-13
 * @version     1.0
 ******************************************************************************/

#include "main.h"

#ifdef EC_APP_SLEEP_TEST

#include "ec_core.h"

#define EC_APP_UART0_TX_BUF_SIZE 1024                 //串口0发送缓冲区大小，可以根据需要调整
#define EC_APP_UART0_RX_BUF_SIZE 1024                 //串口0接收缓冲区大小，可以根据需要调整
uint8_t uart0_tx_buf[EC_APP_UART0_TX_BUF_SIZE] = {0}; //串口0发送缓冲区
uint8_t uart0_rx_buf[EC_APP_UART0_RX_BUF_SIZE] = {0}; //串口0接收缓冲区

void uart0_rx(uint8_t *buf, uint16_t len) //串口0接收数据中断
{
    ec_core_uart_send(EC_CORE_UART0, buf, len); // ECHO 回显
}

void uart0_init(void) //串口0初始化 波特率精度受时钟频率影响
{
    ec_core_uart_init(EC_CORE_UART0, 115200, EC_CORE_UART_PARITY_NONE,
                      EC_CORE_GPIO_P4, EC_CORE_GPIO_P5,
                      uart0_tx_buf, EC_APP_UART0_TX_BUF_SIZE, uart0_rx_buf, EC_APP_UART0_RX_BUF_SIZE,
                      uart0_rx);
}

void int_rising(void) //上升沿中断
{
    ec_core_uart0_printf("sleep_enable\r\n");
    ec_core_sleep_enable(); //睡眠使能
}
void int_falling(void) //下降沿中断
{
    ec_core_uart0_printf("sleep_disable\r\n");
    ec_core_sleep_disable(); //禁止睡眠
}

int main(void)
{
    //演示系统进入睡眠和从睡眠中退出
    //睡眠模式，串口只能发数据，不能收数据，芯片的其他功能正常。
    //非睡眠模式，功耗是毫安级的，3-5mA左右
    //睡眠模式，功耗是微安级的，15-900uA左右，具体参考功耗测试报告

    ec_core_sys_clk_set(EC_CORE_SYS_CLK_48M); //系统时钟初始化

    ec_core_gpio_in_init(EC_CORE_GPIO_P1, EC_CORE_GPIO_PULL_UP_S); // GPIO初始化 上拉输入

    ec_core_init(); //蓝牙内核初始化

    uart0_init();                                  //串口0初始化 波特率精度受时钟频率影响
    ec_core_uart0_printf("ec_app_sleep_test\r\n"); //串口0 printf打印

    ec_core_gpio_int_register(EC_CORE_GPIO_P1, int_rising, int_falling); // GPIO3 中断使能

    ec_core_main_loop_run(); //系统主循环
}

#endif
