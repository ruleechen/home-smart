/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_app_hw_wdg_test.c
 * @brief       hardware watchdog test
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-04-09
 * @version     1.0
 ******************************************************************************/

#include "main.h"

#ifdef EC_APP_HW_WDG_TEST

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

int main(void)
{
    //此例程没有蓝牙功能，下载烧录需要BOOT引脚拉高重新上电强制进入下载模式
    //系统的主循环中已经进行了硬件看门狗喂狗，不需要App参与

    ec_core_sys_clk_set(EC_CORE_SYS_CLK_48M); //系统时钟初始化

    ec_core_init(); //蓝牙内核初始化

    uart0_init(); //串口0初始化 波特率精度受时钟频率影响

    ec_core_uart0_printf("ec_app_hw_wdg_test\r\n"); //串口0 printf打印

    while (1)
    {
        // ec_core_hw_watchdog_feed();     //硬件看门狗喂狗  超时时间是2秒

        ec_core_delay_ms(1000);         //延迟1秒
        ec_core_uart0_printf("hi\r\n"); //打印
    }
}

#endif
