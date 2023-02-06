/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_app_ble_uuid_test.c
 * @brief       uuid demo
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-02-13
 * @version     1.0
 ******************************************************************************/

#include "main.h"

#ifdef EC_APP_BLE_UUID_TEST

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
    ec_core_sys_clk_set(EC_CORE_SYS_CLK_48M); //系统时钟初始化

    // 需要在ec_core_init()之前配置UUID
    // 16bit uuid
    // ec_core_ble_set_suuid("FFF0");//配置服务的UUID 默认FFF0
    // ec_core_ble_set_ruuid("FFF1");//配置读特性的UUID 默认为FFF1
    // ec_core_ble_set_wuuid("FFF2");//配置写特性的UUID 默认为FFF2
    // 128bit uuid
    // ec_core_ble_set_suuid("53527AA4-29F7-AE11-4E74-997334782568");//服务
    // ec_core_ble_set_ruuid("654B749C-E37F-AE1F-EBAB-40CA133E3690");//读特性
    // ec_core_ble_set_wuuid("EE684B1A-1E9B-ED3E-EE55-F894667E92AC");//写特性
    ec_core_ble_set_suuid("0000FFF0-0000-1000-8000-00805F9B34FB");
    ec_core_ble_set_ruuid("0000FFF1-0000-1000-8000-00805F9B34FB");
    ec_core_ble_set_wuuid("0000FFF2-0000-1000-8000-00805F9B34FB");

    ec_core_init(); //蓝牙内核初始化

    uart0_init();                                     //串口0初始化 波特率精度受时钟频率影响
    ec_core_uart0_printf("ec_app_ble_uuid_test\r\n"); //串口0 printf打印

    ec_core_sleep_disable(); //禁止睡眠，串口可以接收数据
    ec_core_main_loop_run(); //系统主循环
}

#endif
