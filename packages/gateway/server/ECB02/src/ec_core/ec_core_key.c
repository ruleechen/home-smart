/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_core_key.c
 * @brief       key control
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-04-09
 * @version     1.0
 ******************************************************************************/

#include "ec_core_key.h"

static ec_core_gpio_pin_e ec_core_key_pin;
static ec_core_sw_timer_e ec_core_key_timer;
static uint8_t ec_core_key_long_press_time = 20; // unit 100MS

static uint8_t ec_core_key_step = 0;
static uint8_t ec_core_key_time = 0;

static ec_core_key_event_t ec_core_key_press_event = 0;
static ec_core_key_event_t ec_core_key_long_press_event = 0;
static ec_core_key_event_t ec_core_key_up_event = 0;

static void ec_core_key_io_rising(void);
static void ec_core_key_io_falling(void);
static void ec_core_key_scan(void);

static void ec_core_key_scan(void)
{
    switch (ec_core_key_step)
    {
    case 0:
        // if (ec_core_gpio_read(ec_core_key_pin) == EC_CORE_GPIO_LEVEL_L)
        // {
        //     ec_core_uart0_printf("key press\r\n");
        //     ec_core_key_step = 1;
        //     ec_core_key_time = 0;
        // }
        // else
        // {
        //     ec_core_key_step = 0;
        //     ec_core_key_time = 0;
        //     ec_core_sw_timer_stop(ec_core_key_timer);
        //     ec_core_gpio_int_register(ec_core_key_pin, ec_app_key_io_rising, ec_app_key_io_falling);
        // }
        // ec_core_uart0_printf("key press\r\n");
        if (ec_core_key_press_event)
            ec_core_key_press_event();
        ec_core_key_step = 1;
        ec_core_key_time = 0;
        break;
    case 1:
        if (ec_core_gpio_read(ec_core_key_pin) == EC_CORE_GPIO_LEVEL_L)
        {
            if (ec_core_key_time <= ec_core_key_long_press_time)
                ec_core_key_time++;
            if (ec_core_key_time == ec_core_key_long_press_time)
            {
                // ec_core_uart0_printf("key long press\r\n");
                if (ec_core_key_long_press_event)
                    ec_core_key_long_press_event();
            }
        }
        else
        {
            // ec_core_uart0_printf("key up\r\n");
            if (ec_core_key_up_event)
                ec_core_key_up_event();
            ec_core_key_step = 0;
            ec_core_key_time = 0;
            ec_core_sw_timer_stop(ec_core_key_timer);
            ec_core_gpio_int_register(ec_core_key_pin, ec_core_key_io_rising, ec_core_key_io_falling);
        }
        break;
    default:
        break;
    }
}

static void ec_core_key_io_rising(void)
{
    // ec_core_uart0_printf("rising\r\n");
}
static void ec_core_key_io_falling(void)
{
    // ec_core_uart0_printf("falling\r\n");
    ec_core_gpio_int_register(ec_core_key_pin, 0, 0); //关闭中断
    ec_core_key_step = 0;
    ec_core_key_time = 0;
    ec_core_sw_timer_stop(ec_core_key_timer);
    ec_core_sw_timer_start(ec_core_key_timer, 100, ec_core_key_scan);
}

void ec_core_key_init(ec_core_gpio_pin_e pin, ec_core_sw_timer_e timer, uint8_t long_press_time_unit_100ms,
                      ec_core_key_event_t press_event, ec_core_key_event_t long_press_event, ec_core_key_event_t up_event)
{
    ec_core_key_pin = pin;
    ec_core_key_timer = timer;
    ec_core_key_long_press_time = long_press_time_unit_100ms;
    ec_core_key_press_event = press_event;
    ec_core_key_long_press_event = long_press_event;
    ec_core_key_up_event = up_event;
    ec_core_gpio_in_init(ec_core_key_pin, EC_CORE_GPIO_PULL_UP_S);
    ec_core_gpio_int_register(ec_core_key_pin, ec_core_key_io_rising, ec_core_key_io_falling);
}
