/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_core_delay.h
 * @brief       software delay
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-02-13
 * @version     1.0
 ******************************************************************************/

#ifndef __EC_CORE_DELAY_H__
#define __EC_CORE_DELAY_H__

#include "stdint.h"

extern void ec_core_delay_ms(uint32_t n); // 毫秒延迟函数 范围0-60000
extern void ec_core_delay_us(uint32_t n); // 微秒延迟函数 范围0-4194301

#endif
