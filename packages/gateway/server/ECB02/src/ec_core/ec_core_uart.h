/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_core_uart.h
 * @brief       uart control
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-02-13
 * @version     1.0
 ******************************************************************************/

#ifndef __EC_CORE_UART_H__
#define __EC_CORE_UART_H__

// 需要退出睡眠模式，串口才能接收数据

// 时钟频率和波特率误差
//                    2400             9600             115200            1000000              2000000
// 64MHz        0.05%/2399       0.14%/9587        0.8%/114286         0%/1000000           0%/2000000
// 48MHz       0.035%/2399        0.2%/9581       0.09%/115108         0%/1000000        37.5%/1454545
// 32MHz        0.01%/2400       0.16%/9615       2.07%/117647         0%/1000000           0%/2000000
// 16MHz        0.11%/2397        0.1%/9610       3.69%/111111         0%/1000000         100%/1000000

#include "stdint.h"

// 有两个串口可以使用
typedef enum
{
    EC_CORE_UART0,
    EC_CORE_UART1,
} ec_core_uart_index_e;

typedef enum
{
    EC_CORE_UART_PARITY_NONE, // 无校验
    EC_CORE_UART_PARITY_EVEN, // even 偶校验
} ec_core_uart_parity_e;

typedef void (*ec_core_uart_int_t)(uint8_t *buf, uint16_t len); //串口中断

// 时钟频率会影响波特率的精度
// 波特率范围:110-2000000
// 串口的TX和RX可以映射到任意GPIO
extern void ec_core_uart_init(ec_core_uart_index_e uart_i,
                              uint32_t baudrate, ec_core_uart_parity_e parity,
                              ec_core_gpio_pin_e tx_pin, ec_core_gpio_pin_e rx_pin,
                              uint8_t *tx_buf, uint16_t tx_buf_size,
                              uint8_t *rx_buf, uint16_t rx_buf_size,
                              ec_core_uart_int_t uart_int);
// 修改串口配置，需要先deinit再init
extern void ec_core_uart_deinit(ec_core_uart_index_e uart_i);
// 使用串口发送数据
extern void ec_core_uart_send(ec_core_uart_index_e uart_i, uint8_t *buf, uint16_t len);

#endif
