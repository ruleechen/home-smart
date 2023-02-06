/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_core_adc.h
 * @brief       adc control
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-02-13
 * @version     1.0
 ******************************************************************************/

#ifndef __EC_CORE_ADC_H__
#define __EC_CORE_ADC_H__

// ADC 基准电压 0.8V
// ADC是12bit的
// 不启用芯片内部分压电阻，ADC量程0-0.8V
// 启用芯片内部的分压电阻，ADC量程0~3.2V
// 不能同时使用内部分压电阻和外部分压电阻

#include "stdint.h"

// adc通道和引脚
typedef enum
{
    EC_CORE_ADC_CH1_P6,
    EC_CORE_ADC_CH2_P7,
    EC_CORE_ADC_CH3_P8,
    EC_CORE_ADC_CH4_P10
} ec_core_adc_ch_e;

// 量程范围
typedef enum
{
    EC_CORE_ADC_RANGE_800MV, // 0V-0.8V
    EC_CORE_ADC_RANGE_3200MV // 0V-3.2V 使用了内部的分压电阻，外部不可以再使用分压电阻
} ec_core_adc_range_e;

// adc校准
typedef enum
{
    EC_CORE_ADC_CALIBRATION_ENABLED,
    EC_CORE_ADC_CALIBRATION_DISABLE
} ec_core_adc_calibration_e;

// 使用adc需要先初始化 放在ec_core_init()之后
extern void ec_core_adc_init(void);

// 获取adc电压
extern void ec_core_adc_get(ec_core_adc_ch_e ch, ec_core_adc_range_e range,
                            ec_core_adc_calibration_e cb, uint16_t *value, uint16_t *voltage);

#endif
