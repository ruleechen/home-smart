/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_core_gpio.h
 * @brief       gpio control
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-02-13
 * @version     1.0
 ******************************************************************************/

// 注意事项
// 每个gpio只能初始化一次，使用ec_core_gpio_out_init或ec_core_gpio_in_init
// 修改gpio方向使用ec_core_gpio_set_dir函数
// 修改上拉或下拉电阻使用ec_core_gpio_set_pull函数
// ec_core_gpio_int_register函数只能在ec_core_init函数之后调用
// 中断函数中不能做太复杂，耗时太久的事情

#ifndef __EC_CORE_GPIO_H__
#define __EC_CORE_GPIO_H__

//一共有11个GPIO可以使用
typedef enum
{
    EC_CORE_GPIO_P0,
    EC_CORE_GPIO_P1,
    EC_CORE_GPIO_P2,
    EC_CORE_GPIO_P3,
    EC_CORE_GPIO_P4,
    EC_CORE_GPIO_P5,
    EC_CORE_GPIO_P6,
    EC_CORE_GPIO_P7,
    EC_CORE_GPIO_P8,
    EC_CORE_GPIO_P9,
    EC_CORE_GPIO_P10,
} ec_core_gpio_pin_e;

// GPIO方向
typedef enum
{
    EC_CORE_GPIO_INPUT,
    EC_CORE_GPIO_OUTPUT
} ec_core_gpio_dir_t;

// 浮空  强上拉  弱上拉  下拉
typedef enum
{
    EC_CORE_GPIO_FLOATING,  // no pull
    EC_CORE_GPIO_PULL_UP_S, // pull up strong 150kΩ
    EC_CORE_GPIO_PULL_UP,   // pull up weak 1MΩ
    EC_CORE_GPIO_PULL_DOWN, // pull down 100kΩ
} ec_core_gpio_pull_e;

typedef enum
{
    EC_CORE_GPIO_LEVEL_L, //低电平
    EC_CORE_GPIO_LEVEL_H  //高电平
} ec_core_gpio_level_e;

typedef void (*ec_core_gpio_int_t)(void); // gpio中断

extern void ec_core_gpio_out_init(ec_core_gpio_pin_e pin, ec_core_gpio_level_e level); //初始化gpio为输出
extern void ec_core_gpio_write(ec_core_gpio_pin_e pin, ec_core_gpio_level_e level);    //设置输出电平
extern void ec_core_gpio_in_init(ec_core_gpio_pin_e pin, ec_core_gpio_pull_e pull);    //初始化GPIO为输入
extern ec_core_gpio_level_e ec_core_gpio_read(ec_core_gpio_pin_e pin);                 //读取GPIO电平
extern void ec_core_gpio_set_dir(ec_core_gpio_pin_e pin, ec_core_gpio_dir_t dir);      //修改GPIO方向
extern void ec_core_gpio_set_pull(ec_core_gpio_pin_e pin, ec_core_gpio_pull_e pull);   //配置GPIO上下拉电阻
//配置GPIO中断
extern void ec_core_gpio_int_register(ec_core_gpio_pin_e pin, ec_core_gpio_int_t int_rising, ec_core_gpio_int_t int_falling);

#endif
