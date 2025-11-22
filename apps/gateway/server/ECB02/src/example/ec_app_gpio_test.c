/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_app_gpio_test.c
 * @brief       gpio demo
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-02-13
 * @version     1.0
 ******************************************************************************/

#include "main.h"

#ifdef EC_APP_GPIO_TEST

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

void int_rising(void) //上升沿中断
{
    ec_core_uart0_printf("int_rising\r\n");
}
void int_falling(void) //下降沿中断
{
    ec_core_uart0_printf("int_falling\r\n");
}

void sw_timer1_int(void) //软件定时器1中断
{
    ec_core_gpio_write(EC_CORE_GPIO_P1, ec_core_gpio_read(EC_CORE_GPIO_P2)); //读取GPIO2的电平，设置给GPIO1
}

int main(void)
{
    ec_core_sys_clk_set(EC_CORE_SYS_CLK_48M); //系统时钟初始化

    ec_core_init(); //蓝牙内核初始化

    ec_core_gpio_out_init(EC_CORE_GPIO_P1, EC_CORE_GPIO_LEVEL_H);  // GPIO1 初始化 输出高电平
    ec_core_gpio_in_init(EC_CORE_GPIO_P2, EC_CORE_GPIO_PULL_UP_S); // GPIO2 初始化 上拉输入
    ec_core_gpio_in_init(EC_CORE_GPIO_P3, EC_CORE_GPIO_PULL_UP_S); // GPIO3 初始化 上拉输入

    uart0_init();                                 //串口0初始化 波特率精度受时钟频率影响
    ec_core_uart0_printf("ec_app_gpio_test\r\n"); //串口0 printf打印

    ec_core_gpio_int_register(EC_CORE_GPIO_P3, int_rising, int_falling); // GPIO3 中断使能

    ec_core_sw_timer_start(EC_CORE_SW_TIMER1, 1000, sw_timer1_int); //开启1秒定时器

    ec_core_sleep_disable(); //禁止睡眠，串口可以接收数据
    ec_core_main_loop_run(); //系统主循环
}

#endif
