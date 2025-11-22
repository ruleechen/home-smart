/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_app_i2c_eeprom_test.c
 * @brief       i2c eeprom demo
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-04-09
 * @version     1.0
 ******************************************************************************/

#include "main.h"

#ifdef EC_APP_I2C_EEPROM_TEST

#include "ec_core.h"

#define EC_APP_EEPROM_ADR 0x50 // EEPROM AT24C02 地址

#define EC_APP_UART0_TX_BUF_SIZE 1024                 //串口0发送缓冲区大小，可以根据需要调整
#define EC_APP_UART0_RX_BUF_SIZE 1024                 //串口0接收缓冲区大小，可以根据需要调整
uint8_t uart0_tx_buf[EC_APP_UART0_TX_BUF_SIZE] = {0}; //串口0发送缓冲区
uint8_t uart0_rx_buf[EC_APP_UART0_RX_BUF_SIZE] = {0}; //串口0接收缓冲区

void uart0_rx(uint8_t *buf, uint16_t len) //串口0接收数据中断
{
    if (buf[0] == 'R') //读取eeprom
    {
        uint8_t temp_buf[16] = {0};
        ec_core_i2c_read_data(EC_APP_EEPROM_ADR, 0, temp_buf, 16); //读取eeprom at24c02  eeprom内部缓冲区只有16字节                                                                  //读取                                                                                     //读取
        ec_core_uart0_printf("eeprom data ");                      //打印
        for (uint8_t i = 0; i < 16; i++)
        {
            ec_core_uart0_printf("%02X", temp_buf[i]); //打印
        }
        ec_core_uart0_printf("\r\n"); //打印
    }
    if (buf[0] == 'W') //写入eeprom
    {
        ec_core_uart0_printf("write eeprom 1234\r\n");

        ec_core_i2c_write_data(EC_APP_EEPROM_ADR, 0, "0123456789123456", 16); // eeprom内部缓冲区只有16字节

        ec_core_delay_ms(20); //写eeprom之后需要延迟一会，再操作eeprom，操作太快eeprom不应答
    }
    if (buf[0] == 'w') //写入eeprom
    {
        ec_core_uart0_printf("write eeprom 4321\r\n");

        ec_core_i2c_write_data(EC_APP_EEPROM_ADR, 0, "9876543210654321", 16); // eeprom内部缓冲区只有16字节

        ec_core_delay_ms(20); //写eeprom之后需要延迟一会，再操作eeprom，操作太快eeprom不应答
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

    uart0_init(); //串口0初始化 波特率精度受时钟频率影响

    ec_core_uart0_printf("ec_app_i2c_eeprom_test\r\n"); //串口0 printf打印

    // I2C EEPROM  AT24C02
    ec_core_i2c_init(EC_CORE_GPIO_P3, EC_CORE_GPIO_P1); //初始化i2c  scl-gpio3  sda-gpio1

    ec_core_sleep_disable(); //禁止睡眠，串口可以接收数据
    ec_core_main_loop_run(); //系统主循环
}

#endif
