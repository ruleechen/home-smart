/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_core_pwm.h
 * @brief       pwm control
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-02-13
 * @version     1.0
 ******************************************************************************/

// 时钟源 16Mhz
// 向上计数
// 任意GPIO都可以输出PWM

#ifndef __EC_CORE_PWM_H__
#define __EC_CORE_PWM_H__

#include "stdint.h"
#include "ec_core_gpio.h"

//最多可以输出6路PWM
typedef enum
{
    EC_CORE_PWM_CH0,
    EC_CORE_PWM_CH1,
    EC_CORE_PWM_CH2,
    EC_CORE_PWM_CH3,
    EC_CORE_PWM_CH4,
    EC_CORE_PWM_CH5
} ec_core_pwm_ch_e;

// PWM分频选项
typedef enum
{
    EC_CORE_PWM_CLK_NO_DIV,
    EC_CORE_PWM_CLK_DIV_2,
    EC_CORE_PWM_CLK_DIV_4,
    EC_CORE_PWM_CLK_DIV_8,
    EC_CORE_PWM_CLK_DIV_16,
    EC_CORE_PWM_CLK_DIV_32,
    EC_CORE_PWM_CLK_DIV_64,
    EC_CORE_PWM_CLK_DIV_128
} ec_core_pwm_clk_div_e;

//启动PWM  分频、通道、GPIO、高电平时间、总时间
extern void ec_core_pwm_start(ec_core_pwm_clk_div_e div, ec_core_pwm_ch_e ch, ec_core_gpio_pin_e pin, uint16_t cmp, uint16_t max);
//关闭PWM
extern void ec_core_pwm_stop(ec_core_pwm_ch_e ch);
//关闭整个PWM模块
extern void ec_core_pwm_close(void);

#endif
