/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_app_manufacturer_data_test.c
 * @brief       manufacturer data demo
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-02-13
 * @version     1.0
 ******************************************************************************/

#include "main.h"

#ifdef EC_APP_MANUFACTURER_DATA_TEST

#include "ec_core.h"

#define EC_APP_UART0_TX_BUF_SIZE 1024                 //串口0发送缓冲区大小，可以根据需要调整
#define EC_APP_UART0_RX_BUF_SIZE 1024                 //串口0接收缓冲区大小，可以根据需要调整
uint8_t uart0_tx_buf[EC_APP_UART0_TX_BUF_SIZE] = {0}; //串口0发送缓冲区
uint8_t uart0_rx_buf[EC_APP_UART0_RX_BUF_SIZE] = {0}; //串口0接收缓冲区

void uart0_rx(uint8_t *buf, uint16_t len) //串口0接收数据中断
{
    if (strcmp((char *)buf, "AA") == 0)
    {
        uint8_t data[3] = {0x34, 0x35, 0x36};
        ec_core_ble_peripheral_set_manufacturer_data(data, 3); //修改厂商自定义数据
        ec_core_ble_peripheral_reset_adv();                    //重新广播后生效
        ec_core_uart0_printf("set bt manufacturer data\r\n");
    }
    if (strcmp((char *)buf, "BB") == 0)
    {
        uint8_t data[22] = {0};
        for (uint8_t i = 0; i < 22; i++)
        {
            data[i] = i;
        }
        ec_core_ble_peripheral_set_manufacturer_data(data, 22); //修改厂商自定义数据
        ec_core_ble_peripheral_reset_adv();                     //重新广播后生效
        ec_core_uart0_printf("set bt manufacturer data\r\n");
    }
    if (strcmp((char *)buf, "CC") == 0)
    {
        ec_core_ble_peripheral_clear_manufacturer_data(); //清除厂商自定义数据
        ec_core_ble_peripheral_reset_adv();               //重新广播后生效
        ec_core_uart0_printf("close manufacturer data\r\n");
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
    //蓝牙广播包中厂商自定义数据，可以实现无连接数据通讯，也可以作为身份识别，过滤蓝牙搜索列表。
    //在苹果手机中，MAC地址属于用户隐私，手机端无法获取到MAC地址，可以把MAC地址写入到厂商自定义数据里面。手机端就可以获取设备的MAC。
    //厂商自定义数据会增加蓝牙广播包的长度，会增加功耗。

    ec_core_sys_clk_set(EC_CORE_SYS_CLK_48M); //系统时钟初始化

    uint8_t data[3] = {0x31, 0x32, 0x33};
    ec_core_ble_peripheral_set_manufacturer_data(data, 3); //初始化厂商自定义数据

    ec_core_init(); //蓝牙内核初始化

    uart0_init();                                              //串口0初始化 波特率精度受时钟频率影响
    ec_core_uart0_printf("set bt manufacturer data test\r\n"); //串口0 printf打印

    ec_core_sleep_disable(); //禁止睡眠，串口可以接收数据
    ec_core_main_loop_run(); //系统主循环
}

#endif
