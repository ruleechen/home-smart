/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_app_beacon_test.c
 * @brief       beacon demo
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-02-13
 * @version     1.0
 ******************************************************************************/

#include "main.h"

#ifdef EC_APP_BEACON_TEST

#include "ec_core.h"

#define EC_APP_UART0_TX_BUF_SIZE 1024                 //串口0发送缓冲区大小，可以根据需要调整
#define EC_APP_UART0_RX_BUF_SIZE 1024                 //串口0接收缓冲区大小，可以根据需要调整
uint8_t uart0_tx_buf[EC_APP_UART0_TX_BUF_SIZE] = {0}; //串口0发送缓冲区
uint8_t uart0_rx_buf[EC_APP_UART0_RX_BUF_SIZE] = {0}; //串口0接收缓冲区

//蓝牙广播包，按照beacon的方式进行广播
uint8_t adv_data[] = {
    // ble flag
    0x02,
    0x01,
    0x06,

    // manufacturer
    0x1a,
    0xff,

    0x4c,
    0x00,
    0x02,
    0x15,

    // device uuid
    0xe2,
    0xc5,
    0x6d,
    0xb5,
    0xdf,
    0xfb,
    0x48,
    0xd2,
    0xb0,
    0x60,
    0xd0,
    0xf5,
    0xa7,
    0x10,
    0x96,
    0xe0,

    // major value
    0x00,
    0x00,
    // minor value
    0x00,
    0x00,

    // power  补码格式
    0xcd,
};

void uart0_rx(uint8_t *buf, uint16_t len) //串口0接收数据中断
{
    if (strcmp((const char *)buf, "AA") == 0) //修改位置数据，重新广播
    {
        adv_data[25] = 0X01;
        ec_core_ble_peripheral_reset_adv();
        ec_core_uart0_printf("set custom adv data\r\n");
    }
    if (strcmp((const char *)buf, "BB") == 0) //修改位置数据，重新广播
    {
        adv_data[25] = 0X02;
        ec_core_ble_peripheral_reset_adv();
        ec_core_uart0_printf("set custom adv data\r\n");
    }
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

    ec_core_ble_peripheral_set_custom_adv_data(adv_data, sizeof(adv_data)); //设置自定义蓝牙广播包
    ec_core_ble_peripheral_set_name("eciot", strlen("eciot"));              //设置蓝牙名称

    ec_core_init();                                     //蓝牙内核初始化
    uart0_init();                                       //串口0初始化 波特率精度受时钟频率影响
    ec_core_uart0_printf("ec_app_custom_adv_test\r\n"); //串口0 printf打印
    ec_core_sleep_disable();                            //禁止睡眠，串口可以接收数据
    ec_core_main_loop_run();                            //系统主循环
}

#endif
