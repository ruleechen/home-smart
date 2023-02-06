/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_core_led.c
 * @brief       led control
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-04-09
 * @version     1.0
 ******************************************************************************/

#include "ec_core_led.h"

static ec_core_sw_timer_e ec_core_led_timer;
static ec_core_gpio_pin_e ec_core_led_pin;
static ec_core_led_mode_e led_mode = EC_CORE_LED_MODE_OFF;
static uint8_t led_time = 0;

static void ec_core_led_on(void)
{
    ec_core_gpio_write(ec_core_led_pin, EC_CORE_GPIO_LEVEL_H);
}
static void ec_core_led_off(void)
{
    ec_core_gpio_write(ec_core_led_pin, EC_CORE_GPIO_LEVEL_L);
}

static void ec_core_led_100ms_task(void)
{
    if (led_mode == EC_CORE_LED_MODE_OFF)
    {
        ec_core_led_off();
    }
    else if (led_mode == EC_CORE_LED_MODE_2S)
    {
        if (++led_time >= 20)
            led_time = 0;
        if (led_time < 3)
            ec_core_led_on();
        else
            ec_core_led_off();
    }
    else if (led_mode == EC_CORE_LED_MODE_1S)
    {
        if (++led_time >= 10)
            led_time = 0;
        if (led_time < 3)
            ec_core_led_on();
        else
            ec_core_led_off();
    }
    else if (led_mode == EC_CORE_LED_MODE_200MS)
    {
        if (++led_time >= 2)
            led_time = 0;
        if (led_time < 1)
            ec_core_led_on();
        else
            ec_core_led_off();
    }
    else if (led_mode == EC_CORE_LED_MODE_ON)
    {
        ec_core_led_on();
    }
}

void ec_core_led_set_mode(ec_core_led_mode_e mode)
{
    led_mode = mode;
}

void ec_core_led_init(ec_core_gpio_pin_e pin, ec_core_sw_timer_e timer)
{
    ec_core_led_pin = pin;
    ec_core_led_timer = timer;
    ec_core_gpio_out_init(ec_core_led_pin, EC_CORE_GPIO_LEVEL_L);
    ec_core_sw_timer_start(ec_core_led_timer, 100, ec_core_led_100ms_task);
}
