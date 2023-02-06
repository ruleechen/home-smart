/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_core_timer.h
 * @brief       timer control
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-02-13
 * @version     1.0
 ******************************************************************************/

#ifndef __EC_CORE_TIMER_H__
#define __EC_CORE_TIMER_H__

//只能在ec_core_init()函数之后，启动定时器
//硬件定时器，时钟源是4MHz，32bit，向下计数，自动重载

#include "stdint.h"

//有六个软件定时器可以使用
typedef enum
{
    EC_CORE_SW_TIMER1,
    EC_CORE_SW_TIMER2,
    EC_CORE_SW_TIMER3,
    EC_CORE_SW_TIMER4,
    EC_CORE_SW_TIMER5,
    EC_CORE_SW_TIMER6,
    EC_CORE_SW_TIMER7,
    EC_CORE_SW_TIMER8,
    EC_CORE_SW_TIMER9,
    EC_CORE_SW_TIMER10,
    EC_CORE_SW_TIMER11,
    EC_CORE_SW_TIMER12,
    EC_CORE_SW_TIMER13,
    EC_CORE_SW_TIMER14,
    EC_CORE_SW_TIMER15,
    EC_CORE_SW_TIMER16,
    EC_CORE_SW_TIMER17,
    EC_CORE_SW_TIMER18,
    EC_CORE_SW_TIMER19,
    EC_CORE_SW_TIMER20,
    EC_CORE_SW_TIMER21,
} ec_core_sw_timer_e;

//有2个硬件定时器可以使用，硬件定时器中断不能做太耗时的工作
typedef enum
{
    EC_CORE_HW_TIMER1,
    EC_CORE_HW_TIMER2,
} ec_core_hw_timer_e;

typedef void (*ec_core_timer_int_t)(void); //定时器中断

extern void ec_core_sw_timer_start(ec_core_sw_timer_e timer, uint32_t ms, ec_core_timer_int_t timer_int); //启动软件定时器
extern void ec_core_sw_timer_stop(ec_core_sw_timer_e timer);                                              //关闭软件定时器

extern void ec_core_hw_timer_init(void);                                                                  //初始化硬件定时器
extern void ec_core_hw_timer_start(ec_core_hw_timer_e timer, uint32_t us, ec_core_timer_int_t timer_int); //启动硬件定时器
extern void ec_core_hw_timer_stop(ec_core_hw_timer_e timer);                                              //关闭硬件定时器
extern uint32_t ec_core_hw_timer_get_count(ec_core_hw_timer_e timer);                                     //获取硬件定时器计数值

#endif
