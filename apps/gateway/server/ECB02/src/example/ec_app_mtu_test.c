/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_app_mtu_test.c
 * @brief       mtu demo
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-02-13
 * @version     1.0
 ******************************************************************************/

#include "main.h"

#ifdef EC_APP_MTU_TEST

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

void sw_timer1_int(void) //软件定时器1中断
{
    ec_core_uart0_printf("link state=%d\r\n", ec_core_ble_get_link_state());                //查询蓝牙连接状态
    ec_core_uart0_printf("notify state=%d\r\n", ec_core_ble_peripheral_get_notify_state()); //查询蓝牙订阅状态
    ec_core_uart0_printf("mtu=%d\r\n", ec_core_ble_get_mtu());                              //查询MTU值
}

int main(void)
{
    // 每个蓝牙数据包的长度最大为MTU值-3
    // 早期蓝牙低功耗协议，MTU为23，所以每个蓝牙数据包长度最大为20字节。
    // ECB02芯片MTU最大支持247，所以每个蓝牙数据包长度最大为244字节。
    // 在蓝牙未连接时，MTU始终为23，此时这个值没有任何意义。
    // 蓝牙连接后，主机和从机会协商出一个MTU值，由主机控制MTU值大小。

    ec_core_sys_clk_set(EC_CORE_SYS_CLK_48M); //系统时钟初始化

    ec_core_init(); //蓝牙内核初始化
    uart0_init();   //串口0初始化 波特率精度受时钟频率影响

    ec_core_uart0_printf("link state=%d\r\n", ec_core_ble_get_link_state());                //查询蓝牙连接状态
    ec_core_uart0_printf("notify state=%d\r\n", ec_core_ble_peripheral_get_notify_state()); //查询蓝牙订阅状态
    ec_core_uart0_printf("mtu=%d\r\n", ec_core_ble_get_mtu());                              //查询MTU值

    ec_core_sw_timer_start(EC_CORE_SW_TIMER1, 1000, sw_timer1_int); //启动软件定时器1,周期1秒

    ec_core_sleep_disable(); //禁止睡眠，串口可以接收数据
    ec_core_main_loop_run(); //系统主循环
}

#endif
