/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_core_spi_ssd1306_oled.h
 * @brief       spi ssd1306 oled driver
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-04-09
 * @version     1.0
 ******************************************************************************/

#ifndef __EC_CORE_SPI_SSD1306_OLED_H__
#define __EC_CORE_SPI_SSD1306_OLED_H__

#include "stdint.h"
#include "ec_core_gpio.h"

extern void ec_core_spi_ssd1306_oled_init(ec_core_gpio_pin_e sckl, ec_core_gpio_pin_e mosi, ec_core_gpio_pin_e rst, ec_core_gpio_pin_e dc);
extern void ec_core_spi_ssd1306_oled_show_string(uint8_t x, uint8_t y, uint8_t *str);

#endif
