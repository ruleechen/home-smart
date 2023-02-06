/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_app_custom_adv_test.c
 * @brief       custom advert demo
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-02-13
 * @version     1.0
 ******************************************************************************/

#include "main.h"

#ifdef EC_APP_CUSTOM_ADV_TEST

#include "ec_core.h"

#define EC_APP_UART0_TX_BUF_SIZE 1024                 //串口0发送缓冲区大小，可以根据需要调整
#define EC_APP_UART0_RX_BUF_SIZE 1024                 //串口0接收缓冲区大小，可以根据需要调整
uint8_t uart0_tx_buf[EC_APP_UART0_TX_BUF_SIZE] = {0}; //串口0发送缓冲区
uint8_t uart0_rx_buf[EC_APP_UART0_RX_BUF_SIZE] = {0}; //串口0接收缓冲区

//自定义广播包
uint8_t adv_data[] = {
    0x02, 0x01, 0x06,      // ble flag
    0x03, 0x02, 0xf0, 0xff // server
};

//自定义扫描应答包
uint8_t scan_rsp[] = {
    0x06,
    0x09,
    'E',
    'C',
    'I',
    'O',
    'T',
};

void uart0_rx(uint8_t *buf, uint16_t len) //串口0接收数据中断
{
    if (strcmp((const char *)buf, "AA") == 0) //修改广播包内容，重新广播
    {
        adv_data[5] = 0XE0;
        ec_core_ble_peripheral_set_custom_adv_data(adv_data, sizeof(adv_data));
        ec_core_ble_peripheral_reset_adv();
        ec_core_uart0_printf("set custom adv data\r\n");
    }
    if (strcmp((const char *)buf, "BB") == 0) //关闭自定义广播包，使用系统默认的
    {
        ec_core_ble_peripheral_clear_custom_adv_data();
        ec_core_ble_peripheral_reset_adv();
        ec_core_uart0_printf("clear custom adv data\r\n");
    }
    if (strcmp((const char *)buf, "CC") == 0) //设置自定义扫描应答包
    {
        uint8_t temp_scan_rsp[] = {0x09, 0x09, 'E', 'C', 'I', 'O', 'T', '_', 'B', 'T'};
        ec_core_ble_peripheral_set_custom_scan_rsp(temp_scan_rsp, sizeof(temp_scan_rsp));
        ec_core_ble_peripheral_reset_adv();
        ec_core_uart0_printf("set custom scan rsp\r\n");
    }
    if (strcmp((const char *)buf, "DD") == 0) //关闭自定义的扫描应答包，使用系统默认的
    {
        ec_core_ble_peripheral_clear_custom_scan_rsp();
        ec_core_ble_peripheral_reset_adv();
        ec_core_uart0_printf("clear custom scan rsp\r\n");
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

    ec_core_ble_peripheral_set_custom_adv_data(adv_data, sizeof(adv_data)); //初始化默认自定义广播包
    ec_core_ble_peripheral_set_custom_scan_rsp(scan_rsp, sizeof(scan_rsp)); //初始化默认自定义扫描应答包

    ec_core_init(); //蓝牙内核初始化

    uart0_init();                                       //串口0初始化 波特率精度受时钟频率影响
    ec_core_uart0_printf("ec_app_custom_adv_test\r\n"); //串口0 printf打印

    ec_core_sleep_disable(); //禁止睡眠，串口可以接收数据
    ec_core_main_loop_run(); //系统主循环
}

#endif
