/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_app_adc_test.c
 * @brief       adc demo
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-02-13
 * @version     1.0
 ******************************************************************************/

#include "main.h"

#ifdef EC_APP_ADC_TEST

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
    uint16_t value, voltage; //寄存器值和电压值
    ec_core_adc_get(EC_CORE_ADC_CH1_P6, EC_CORE_ADC_RANGE_3200MV, EC_CORE_ADC_CALIBRATION_ENABLED, &value, &voltage);
    ec_core_uart0_printf("value=%d,voltage=%d\r\n", value, voltage);

    // uint16_t value, voltage; //寄存器值和电压值
    // ec_core_adc_get(EC_CORE_ADC_CH1_P6, EC_CORE_ADC_RANGE_3200MV, EC_CORE_ADC_CALIBRATION_ENABLED, &value, &voltage);
    // ec_core_uart0_printf("value=%d,voltage=%d||", value, voltage);
    // ec_core_adc_get(EC_CORE_ADC_CH2_P7, EC_CORE_ADC_RANGE_3200MV, EC_CORE_ADC_CALIBRATION_ENABLED, &value, &voltage);
    // ec_core_uart0_printf("value=%d,voltage=%d||", value, voltage);
    // ec_core_adc_get(EC_CORE_ADC_CH3_P8, EC_CORE_ADC_RANGE_3200MV, EC_CORE_ADC_CALIBRATION_ENABLED, &value, &voltage);
    // ec_core_uart0_printf("value=%d,voltage=%d||", value, voltage);
    // ec_core_adc_get(EC_CORE_ADC_CH4_P10, EC_CORE_ADC_RANGE_3200MV, EC_CORE_ADC_CALIBRATION_ENABLED, &value, &voltage);
    // ec_core_uart0_printf("value=%d,voltage=%d\r\n", value, voltage);
}

int main(void)
{
    ec_core_sys_clk_set(EC_CORE_SYS_CLK_48M); //系统时钟初始化

    ec_core_init(); //蓝牙内核初始化

    uart0_init();                                //串口0初始化 波特率精度受时钟频率影响
    ec_core_uart0_printf("ec_app_adc_test\r\n"); //串口0 printf打印

    ec_core_adc_init();                                             //使用adc前，需要先初始化，需要放在ec_core_init()之后。
    ec_core_sw_timer_start(EC_CORE_SW_TIMER1, 1000, sw_timer1_int); //启动软件定时器1,周期1秒

    // ec_core_sleep_disable(); //禁止睡眠，串口可以接收数据
    ec_core_sleep_enable();
    ec_core_main_loop_run(); //系统主循环
}

#endif
