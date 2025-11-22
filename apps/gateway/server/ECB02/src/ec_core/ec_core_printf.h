/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_core_printf.h
 * @brief       printf function
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-02-13
 * @version     1.0
 ******************************************************************************/

#ifndef __EC_CORE_PRINTF_H__
#define __EC_CORE_PRINTF_H__

//通过串口0进行printf打印
extern void ec_core_uart0_printf(const char *format, ...);
//通过串口1进行printf打印
extern void ec_core_uart1_printf(const char *format, ...);
//通过蓝牙进行printf打印
extern void ec_core_ble_printf(const char *format, ...);

#endif
