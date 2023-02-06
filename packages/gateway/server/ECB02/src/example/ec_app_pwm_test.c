/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_app_pwm_test.c
 * @brief       pwm demo
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-02-13
 * @version     1.0
 ******************************************************************************/

#include "main.h"

#ifdef EC_APP_PWM_TEST

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

uint8_t ec_app_strcmp(uint8_t *p1, uint8_t p1_len, char *p2)
{
    uint8_t max = strlen(p2);
    if (p1_len != max)
        return 1;
    for (uint8_t n = 0; n < max; n++)
    {
        if (p1[n] != p2[n])
            return 1;
    }
    return 0;
}

static void ec_app_ble_peripheral_connect_event(void) //蓝牙连接成功回调
{
    ec_core_uart0_printf("ble peripheral connect\r\n");
}
static void ec_app_ble_peripheral_disconnect_event(void) //蓝牙断开连接回调
{
    ec_core_uart0_printf("ble peripheral disconnect\r\n");
}
static void ec_app_ble_peripheral_notify_enable_event(void) //订阅打开回调
{
    ec_core_uart0_printf("ble peripheral notify enable\r\n");
}
static void ec_app_ble_peripheral_notify_disable_event(void) //订阅关闭回调
{
    ec_core_uart0_printf("ble peripheral notify disable\r\n");
}
static void ec_app_ble_peripheral_receive_event(uint8_t *data, uint8_t len) //蓝牙接收数据回调
{
    ec_core_uart0_printf("ble peripheral receive len=%d\r\n", len);
    ec_core_uart_send(EC_CORE_UART0, data, len);
    ec_core_uart0_printf("\r\n");

    if (ec_app_strcmp(data, len, "11") == 0)
    {
        ec_core_pwm_start(EC_CORE_PWM_CLK_NO_DIV, EC_CORE_PWM_CH0, EC_CORE_GPIO_P1, 500, 1000); // 占空比50% 频率=16Mhz/1000=16KHz
    }
    if (ec_app_strcmp(data, len, "12") == 0)
    {
        ec_core_pwm_start(EC_CORE_PWM_CLK_NO_DIV, EC_CORE_PWM_CH0, EC_CORE_GPIO_P1, 30, 100); // 占空比30% 频率=16Mhz/100=160KHz
    }
    if (ec_app_strcmp(data, len, "13") == 0)
    {
        ec_core_pwm_start(EC_CORE_PWM_CLK_NO_DIV, EC_CORE_PWM_CH0, EC_CORE_GPIO_P1, 7, 10); // 占空比70% 频率=16Mhz/10=1.6MHz
    }
    if (ec_app_strcmp(data, len, "14") == 0)
    {
        ec_core_pwm_stop(EC_CORE_PWM_CH0);
    }

    if (ec_app_strcmp(data, len, "21") == 0)
    {
        ec_core_pwm_start(EC_CORE_PWM_CLK_NO_DIV, EC_CORE_PWM_CH1, EC_CORE_GPIO_P2, 500, 1000); // 占空比50% 频率=16Mhz/1000=16KHz
    }
    if (ec_app_strcmp(data, len, "22") == 0)
    {
        ec_core_pwm_start(EC_CORE_PWM_CLK_NO_DIV, EC_CORE_PWM_CH1, EC_CORE_GPIO_P2, 30, 100); // 占空比30% 频率=16Mhz/100=160KHz
    }
    if (ec_app_strcmp(data, len, "23") == 0)
    {
        ec_core_pwm_start(EC_CORE_PWM_CLK_NO_DIV, EC_CORE_PWM_CH1, EC_CORE_GPIO_P2, 7, 10); // 占空比70% 频率=16Mhz/10=1.6MHz
    }
    if (ec_app_strcmp(data, len, "24") == 0)
    {
        ec_core_pwm_stop(EC_CORE_PWM_CH1);
    }

    if (ec_app_strcmp(data, len, "31") == 0)
    {
        ec_core_pwm_start(EC_CORE_PWM_CLK_NO_DIV, EC_CORE_PWM_CH2, EC_CORE_GPIO_P3, 500, 1000); // 占空比50% 频率=16Mhz/1000=16KHz
    }
    if (ec_app_strcmp(data, len, "32") == 0)
    {
        ec_core_pwm_start(EC_CORE_PWM_CLK_NO_DIV, EC_CORE_PWM_CH2, EC_CORE_GPIO_P3, 30, 100); // 占空比30% 频率=16Mhz/100=160KHz
    }
    if (ec_app_strcmp(data, len, "33") == 0)
    {
        ec_core_pwm_start(EC_CORE_PWM_CLK_NO_DIV, EC_CORE_PWM_CH2, EC_CORE_GPIO_P3, 7, 10); // 占空比70% 频率=16Mhz/10=1.6MHz
    }
    if (ec_app_strcmp(data, len, "34") == 0)
    {
        ec_core_pwm_stop(EC_CORE_PWM_CH2);
    }

    if (ec_app_strcmp(data, len, "41") == 0)
    {
        ec_core_pwm_start(EC_CORE_PWM_CLK_NO_DIV, EC_CORE_PWM_CH3, EC_CORE_GPIO_P6, 500, 1000); // 占空比50% 频率=16Mhz/1000=16KHz
    }
    if (ec_app_strcmp(data, len, "42") == 0)
    {
        ec_core_pwm_start(EC_CORE_PWM_CLK_NO_DIV, EC_CORE_PWM_CH3, EC_CORE_GPIO_P6, 30, 100); // 占空比30% 频率=16Mhz/100=160KHz
    }
    if (ec_app_strcmp(data, len, "43") == 0)
    {
        ec_core_pwm_start(EC_CORE_PWM_CLK_NO_DIV, EC_CORE_PWM_CH3, EC_CORE_GPIO_P6, 7, 10); // 占空比70% 频率=16Mhz/10=1.6MHz
    }
    if (ec_app_strcmp(data, len, "44") == 0)
    {
        ec_core_pwm_stop(EC_CORE_PWM_CH3);
    }

    if (ec_app_strcmp(data, len, "51") == 0)
    {
        ec_core_pwm_start(EC_CORE_PWM_CLK_NO_DIV, EC_CORE_PWM_CH4, EC_CORE_GPIO_P7, 500, 1000); // 占空比50% 频率=16Mhz/1000=16KHz
    }
    if (ec_app_strcmp(data, len, "52") == 0)
    {
        ec_core_pwm_start(EC_CORE_PWM_CLK_NO_DIV, EC_CORE_PWM_CH4, EC_CORE_GPIO_P7, 30, 100); // 占空比30% 频率=16Mhz/100=160KHz
    }
    if (ec_app_strcmp(data, len, "53") == 0)
    {
        ec_core_pwm_start(EC_CORE_PWM_CLK_NO_DIV, EC_CORE_PWM_CH4, EC_CORE_GPIO_P7, 7, 10); // 占空比70% 频率=16Mhz/10=1.6MHz
    }
    if (ec_app_strcmp(data, len, "54") == 0)
    {
        ec_core_pwm_stop(EC_CORE_PWM_CH4);
    }

    if (ec_app_strcmp(data, len, "61") == 0)
    {
        ec_core_pwm_start(EC_CORE_PWM_CLK_NO_DIV, EC_CORE_PWM_CH5, EC_CORE_GPIO_P8, 500, 1000); // 占空比50% 频率=16Mhz/1000=16KHz
    }
    if (ec_app_strcmp(data, len, "62") == 0)
    {
        ec_core_pwm_start(EC_CORE_PWM_CLK_NO_DIV, EC_CORE_PWM_CH5, EC_CORE_GPIO_P8, 30, 100); // 占空比30% 频率=16Mhz/100=160KHz
    }
    if (ec_app_strcmp(data, len, "63") == 0)
    {
        ec_core_pwm_start(EC_CORE_PWM_CLK_NO_DIV, EC_CORE_PWM_CH5, EC_CORE_GPIO_P8, 7, 10); // 占空比70% 频率=16Mhz/10=1.6MHz
    }
    if (ec_app_strcmp(data, len, "64") == 0)
    {
        ec_core_pwm_stop(EC_CORE_PWM_CH5);
    }

    if (ec_app_strcmp(data, len, "off") == 0)
    {
        ec_core_pwm_close(); //关闭整个PWM模块
    }
}

int main(void)
{
    ec_core_sys_clk_set(EC_CORE_SYS_CLK_48M); //系统时钟初始化

    ec_core_ble_peripheral_set_event_cb(EC_CORE_BLE_PERIPHERAL_EVENT_CONNECT, ec_app_ble_peripheral_connect_event);               //注册蓝牙连接的回调
    ec_core_ble_peripheral_set_event_cb(EC_CORE_BLE_PERIPHERAL_EVENT_DISCONNECT, ec_app_ble_peripheral_disconnect_event);         //注册蓝牙断开的回调
    ec_core_ble_peripheral_set_event_cb(EC_CORE_BLE_PERIPHERAL_EVENT_NOTIFY_ENABLE, ec_app_ble_peripheral_notify_enable_event);   //注册蓝牙订阅打开的回调
    ec_core_ble_peripheral_set_event_cb(EC_CORE_BLE_PERIPHERAL_EVENT_NOTIFY_DISABLE, ec_app_ble_peripheral_notify_disable_event); //注册蓝牙订阅关闭的回调
    ec_core_ble_peripheral_set_receive_cb(ec_app_ble_peripheral_receive_event);                                                   //注册蓝牙数据接收的回调

    ec_core_init(); //蓝牙内核初始化

    uart0_init();                                //串口0初始化 波特率精度受时钟频率影响
    ec_core_uart0_printf("ec_app_pwm_test\r\n"); //串口0 printf打印

    //最多可以输出6路PWM 可以映射在任意引脚 需要在ec_core_init()之后调用
    ec_core_pwm_start(EC_CORE_PWM_CLK_NO_DIV, EC_CORE_PWM_CH0, EC_CORE_GPIO_P1, 500, 1000); // 占空比50% 频率=16Mhz/1000=16KHz
    ec_core_pwm_start(EC_CORE_PWM_CLK_NO_DIV, EC_CORE_PWM_CH1, EC_CORE_GPIO_P2, 500, 1000); // 占空比50% 频率=16Mhz/1000=16KHz
    ec_core_pwm_start(EC_CORE_PWM_CLK_NO_DIV, EC_CORE_PWM_CH2, EC_CORE_GPIO_P3, 500, 1000); // 占空比50% 频率=16Mhz/1000=16KHz
    ec_core_pwm_start(EC_CORE_PWM_CLK_NO_DIV, EC_CORE_PWM_CH3, EC_CORE_GPIO_P6, 500, 1000); // 占空比50% 频率=16Mhz/1000=16KHz
    ec_core_pwm_start(EC_CORE_PWM_CLK_NO_DIV, EC_CORE_PWM_CH4, EC_CORE_GPIO_P7, 500, 1000); // 占空比50% 频率=16Mhz/1000=16KHz
    ec_core_pwm_start(EC_CORE_PWM_CLK_NO_DIV, EC_CORE_PWM_CH5, EC_CORE_GPIO_P8, 500, 1000); // 占空比50% 频率=16Mhz/1000=16KHz

    // ec_core_sleep_disable(); //禁止睡眠，串口可以接收数据
    ec_core_sleep_enable();
    ec_core_main_loop_run(); //系统主循环
}

#endif
