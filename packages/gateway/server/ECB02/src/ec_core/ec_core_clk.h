/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_core_clk.h
 * @brief       set system clk
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-02-13
 * @version     1.0
 ******************************************************************************/

#ifndef __EC_CORE_CLK_H__
#define __EC_CORE_CLK_H__

//系统时钟频率
typedef enum
{
    EC_CORE_SYS_CLK_16M,
    EC_CORE_SYS_CLK_32M,
    EC_CORE_SYS_CLK_48M,
    EC_CORE_SYS_CLK_64M,
} ec_core_sysclk_t;

extern void ec_core_sys_clk_set(ec_core_sysclk_t p); //配置时钟频率  放在ec_core_init()之前

#endif
