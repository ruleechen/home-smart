/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_app_name_test.c
 * @brief       ble name demo
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-02-13
 * @version     1.0
 ******************************************************************************/

#include "main.h"

#ifdef EC_APP_NAME_TEST

#include "ec_core.h"
#include "string.h"

#define BT_NAME_TEST "BT_TEST"
#define BT_NAME_TEST2 "TEST2_BT"
#define BT_NAME_TEST3 "TEST3_BT"

#define EC_APP_UART0_TX_BUF_SIZE 1024                 //串口0发送缓冲区大小，可以根据需要调整
#define EC_APP_UART0_RX_BUF_SIZE 1024                 //串口0接收缓冲区大小，可以根据需要调整
uint8_t uart0_tx_buf[EC_APP_UART0_TX_BUF_SIZE] = {0}; //串口0发送缓冲区
uint8_t uart0_rx_buf[EC_APP_UART0_RX_BUF_SIZE] = {0}; //串口0接收缓冲区

void uart0_rx(uint8_t *buf, uint16_t len) //串口0接收数据中断
{
    if (strcmp((char *)buf, "TEST") == 0)
    {
        ec_core_ble_peripheral_set_name(BT_NAME_TEST2, strlen(BT_NAME_TEST2)); //修改蓝牙名字
        ec_core_ble_peripheral_reset_adv();                                    //重新广播后生效
        ec_core_uart0_printf("set bt name test\r\n");
    }
    if (strcmp((char *)buf, "TESTCN") == 0)
    {
        // 0XE69893  易
        // 0XE58AA0  加
        // 0XE789A9  物
        // 0XE88194  联
        uint8_t name_cn[] = {
            0xe6, 0x98, 0x93,
            0xe5, 0x8a, 0xa0,
            0xe7, 0x89, 0xa9,
            0xe8, 0x81, 0x94};
        ec_core_ble_peripheral_set_name(name_cn, 4 * 3); //修改蓝牙名字为中文
        ec_core_ble_peripheral_reset_adv();              //重新广播后生效
        ec_core_uart0_printf("set bt name cn test\r\n");
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
    ec_core_sw_timer_stop(EC_CORE_SW_TIMER1);                              //停止定时器
    ec_core_ble_peripheral_set_name(BT_NAME_TEST3, strlen(BT_NAME_TEST3)); //修改蓝牙名字
    ec_core_ble_peripheral_reset_adv();                                    //重新广播后生效
    ec_core_uart0_printf("set bt name test\r\n");
}

int main(void)
{
    ec_core_sys_clk_set(EC_CORE_SYS_CLK_48M); //系统时钟初始化

    ec_core_ble_peripheral_set_name(BT_NAME_TEST, strlen(BT_NAME_TEST)); //修改默认的蓝牙名字 最长20字节

    ec_core_init(); //蓝牙内核初始化

    uart0_init();                                 //串口0初始化 波特率精度受时钟频率影响
    ec_core_uart0_printf("set bt name test\r\n"); //串口0 printf打印

    ec_core_sw_timer_start(EC_CORE_SW_TIMER1, 10000, sw_timer1_int); //启动软件定时器1,周期10秒

    ec_core_sleep_disable(); //禁止睡眠，串口可以接收数据
    ec_core_main_loop_run(); //系统主循环
}

#endif
