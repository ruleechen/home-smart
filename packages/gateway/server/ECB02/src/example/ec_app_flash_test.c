/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_app_flash_test.c
 * @brief       flash demo
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-02-13
 * @version     1.0
 ******************************************************************************/

#include "main.h"

#ifdef EC_APP_FLASH_TEST

#include "ec_core.h"

#define FLASH_ADDR_TEST 0x3F000 // flash最后一个扇区

#define EC_APP_UART0_TX_BUF_SIZE 1024                 //串口0发送缓冲区大小，可以根据需要调整
#define EC_APP_UART0_RX_BUF_SIZE 1024                 //串口0接收缓冲区大小，可以根据需要调整
uint8_t uart0_tx_buf[EC_APP_UART0_TX_BUF_SIZE] = {0}; //串口0发送缓冲区
uint8_t uart0_rx_buf[EC_APP_UART0_RX_BUF_SIZE] = {0}; //串口0接收缓冲区

void uart0_rx(uint8_t *buf, uint16_t len) //串口0接收数据中断
{
    if (buf[0] == 'R') //读取flash
    {
        uint8_t temp_buf[4] = {0};
        ec_core_flash_read(FLASH_ADDR_TEST, temp_buf, 4);                                                                                        //读取
        ec_core_uart0_printf("flash addr 0x%05X - 0x%02X%02X%02X%02X\r\n", FLASH_ADDR_TEST, temp_buf[0], temp_buf[1], temp_buf[2], temp_buf[3]); //打印
    }
    if (buf[0] == 'W') //写入flash
    {
        ec_core_uart0_printf("write flash 1234\r\n");
        ec_core_delay_ms(20);                                       //延迟20ms 操作flash之前先延迟一会 避免和串口冲突
        ec_core_flash_erase_sector(FLASH_ADDR_TEST);                //擦除flash扇区
        ec_core_flash_write(FLASH_ADDR_TEST, (uint8_t *)"1234", 4); //写入数据
    }
    if (buf[0] == 'w') //写入flash
    {
        ec_core_uart0_printf("write flash 4321\r\n");
        ec_core_delay_ms(20);                                       //延迟20ms 操作flash之前先延迟一会 避免和串口冲突
        ec_core_flash_erase_sector(FLASH_ADDR_TEST);                //擦除flash扇区
        ec_core_flash_write(FLASH_ADDR_TEST, (uint8_t *)"4321", 4); //写入数据
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

    ec_core_init(); //蓝牙内核初始化

    uart0_init();                           //串口0初始化 波特率精度受时钟频率影响
    ec_core_uart0_printf("flash test\r\n"); //串口0 printf打印

    ec_core_sleep_disable(); //禁止睡眠，串口可以接收数据
    ec_core_main_loop_run(); //系统主循环
}

#endif
