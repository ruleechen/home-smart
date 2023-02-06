/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_core_flash.h
 * @brief       flash control
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-02-13
 * @version     1.0
 ******************************************************************************/

#ifndef __EC_CORE_FLASH_H__
#define __EC_CORE_FLASH_H__

// flash大小  256KB
// 64个扇区 1个扇区4KB
// 地址范围   0x00000 --- 0x3FFFF
// 0x00000 --- 0x10FFF  BOOT
// 0x11000 --- 0x1FFFF  APP
// 0x20000 --- 0x3FFFF  APP XIP

// flash需要按扇区操作
// 写flash需要先擦除再写入
// 写flash的过程中，如果突然断电，数据可能会丢失。需要写一些安全的机制。

#include "stdint.h"

extern void ec_core_flash_erase_sector(uint32_t addr);                       //擦除flash扇区
extern void ec_core_flash_write(uint32_t addr, uint8_t *data, uint16_t len); //数据写入，需要扇区对齐
extern void ec_core_flash_read(uint32_t addr, uint8_t *data, uint16_t len);  //数据读取，需要扇区对齐

#endif
