/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_app_timer_test.c
 * @brief       timer demo
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-02-13
 * @version     1.0
 ******************************************************************************/

#include "main.h"

#ifdef EC_APP_TIMER_TEST

#include "ec_core.h"

#define EC_APP_UART0_TX_BUF_SIZE 1024                 //串口0发送缓冲区大小，可以根据需要调整
#define EC_APP_UART0_RX_BUF_SIZE 1024                 //串口0接收缓冲区大小，可以根据需要调整
uint8_t uart0_tx_buf[EC_APP_UART0_TX_BUF_SIZE] = {0}; //串口0发送缓冲区
uint8_t uart0_rx_buf[EC_APP_UART0_RX_BUF_SIZE] = {0}; //串口0接收缓冲区

ec_core_sw_timer_e timer_test = EC_CORE_SW_TIMER21;
void sw_timer_test(void)
{
    ec_core_uart0_printf("sw_timer_test %d\r\n", timer_test);
}
void uart0_rx(uint8_t *buf, uint16_t len) //串口0接收数据中断
{
    if (buf[0] == 'A')
    {
        ec_core_sw_timer_start(timer_test, 1000, sw_timer_test);
    }
    if (buf[0] == 'B')
    {
        ec_core_sw_timer_stop(timer_test);
    }
}

void uart0_init(void) //串口0初始化 波特率精度受时钟频率影响
{
    ec_core_uart_init(EC_CORE_UART0, 115200, EC_CORE_UART_PARITY_NONE,
                      EC_CORE_GPIO_P4, EC_CORE_GPIO_P5,
                      uart0_tx_buf, EC_APP_UART0_TX_BUF_SIZE, uart0_rx_buf, EC_APP_UART0_RX_BUF_SIZE,
                      uart0_rx);
}

void sw_timer1_int(void) //软件定时器1中断
{
    // ec_core_uart0_printf("sw_timer1_int\r\n");
}
void hw_timer1_int(void) //硬件定时器1中断
{
    // ec_core_uart0_printf("hw_timer1_int\r\n");
}
int main(void)
{
    ec_core_sys_clk_set(EC_CORE_SYS_CLK_48M); //系统时钟初始化

    ec_core_init(); //蓝牙内核初始化

    uart0_init();                                  //串口0初始化 波特率精度受时钟频率影响
    ec_core_uart0_printf("ec_app_timer_test\r\n"); //串口0 printf打印

    ec_core_sw_timer_start(EC_CORE_SW_TIMER1, 1000, sw_timer1_int); //软件定时器 一共有6个可以使用 需要在ec_core_init()之后调用

    ec_core_hw_timer_init();                                           //初始化硬件定时器 一共有2个可以使用，需要在ec_core_init()之后调用
    ec_core_hw_timer_start(EC_CORE_HW_TIMER1, 1000000, hw_timer1_int); //中断里面不能做太耗时间的工作

    ec_core_sleep_disable(); //禁止睡眠，串口可以接收数据
    ec_core_main_loop_run(); //系统主循环
}

#endif
