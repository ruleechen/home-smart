/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_core_i2c.h
 * @brief       i2c control
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-04-09
 * @version     1.0
 ******************************************************************************/

#ifndef __EC_CORE_I2C_H__
#define __EC_CORE_I2C_H__

#include "stdint.h"
#include "ec_core_gpio.h"

extern void ec_core_i2c_init(ec_core_gpio_pin_e scl, ec_core_gpio_pin_e sda);

extern void ec_core_i2c_write_data(uint8_t dev_addr_7, uint8_t addr, uint8_t *buf, uint8_t size);
extern void ec_core_i2c_write_one_byte_data(uint8_t dev_addr_7, uint8_t addr, uint8_t buf);
extern void ec_core_i2c_write_data_16bit_addr(uint8_t dev_addr_7, uint16_t addr, uint8_t *buf, uint8_t size);
extern void ec_core_i2c_write_one_byte_data_16bit_addr(uint8_t dev_addr_7, uint16_t addr, uint8_t buf);

extern void ec_core_i2c_read_data(uint8_t dev_addr_7, uint8_t addr, uint8_t *buf, uint8_t size);
extern uint8_t ec_core_i2c_read_one_byte_data(uint8_t dev_addr_7, uint8_t addr);
extern void ec_core_i2c_read_data_16bit_addr(uint8_t dev_addr_7, uint16_t addr, uint8_t *buf, uint8_t size);
extern uint8_t ec_core_i2c_read_one_byte_data_16bit_addr(uint8_t dev_addr_7, uint16_t addr);

#endif
