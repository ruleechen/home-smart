/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_core_i2c.c
 * @brief       i2c control
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-04-09
 * @version     1.0
 ******************************************************************************/

#include "stdint.h"
#include "ec_core_gpio.h"
#include "ec_core_delay.h"
#include "ec_core_i2c.h"

static ec_core_gpio_pin_e ec_core_i2c_scl_pin;
static ec_core_gpio_pin_e ec_core_i2c_sda_pin;

static void ec_core_i2c_delay(uint8_t n)
{
    ec_core_delay_us(n);
}

void ec_core_i2c_init(ec_core_gpio_pin_e scl, ec_core_gpio_pin_e sda)
{
    ec_core_i2c_scl_pin = scl;
    ec_core_i2c_sda_pin = sda;

    ec_core_gpio_out_init(ec_core_i2c_scl_pin, EC_CORE_GPIO_LEVEL_H);

    ec_core_gpio_out_init(ec_core_i2c_sda_pin, EC_CORE_GPIO_LEVEL_H);
    ec_core_gpio_set_pull(ec_core_i2c_sda_pin, EC_CORE_GPIO_PULL_UP_S);
}
static void ec_core_i2c_scl_h(void)
{
    ec_core_gpio_write(ec_core_i2c_scl_pin, EC_CORE_GPIO_LEVEL_H);
}
static void ec_core_i2c_scl_l(void)
{
    ec_core_gpio_write(ec_core_i2c_scl_pin, EC_CORE_GPIO_LEVEL_L);
}
static void ec_core_i2c_sda_h(void)
{
    ec_core_gpio_write(ec_core_i2c_sda_pin, EC_CORE_GPIO_LEVEL_H);
}
static void ec_core_i2c_sda_l(void)
{
    ec_core_gpio_write(ec_core_i2c_sda_pin, EC_CORE_GPIO_LEVEL_L);
}
static void ec_core_i2c_sda_in(void)
{
    ec_core_gpio_set_dir(ec_core_i2c_sda_pin, EC_CORE_GPIO_INPUT);
}
static void ec_core_i2c_sda_out(void)
{
    ec_core_gpio_set_dir(ec_core_i2c_sda_pin, EC_CORE_GPIO_OUTPUT);
}
static ec_core_gpio_level_e ec_core_i2c_sda_read(void)
{
    return ec_core_gpio_read(ec_core_i2c_sda_pin);
}

static void ec_core_i2c_start(void)
{
    ec_core_i2c_sda_h();
    ec_core_i2c_scl_h();
    ec_core_i2c_delay(10);
    ec_core_i2c_sda_l();
    ec_core_i2c_delay(10);
    ec_core_i2c_scl_l();
}

static void ec_core_i2c_stop(void)
{
    ec_core_i2c_sda_l();
    ec_core_i2c_delay(10);
    ec_core_i2c_scl_h();
    ec_core_i2c_delay(10);
    ec_core_i2c_sda_h();
    ec_core_i2c_delay(10);
}

static uint8_t ec_core_i2c_send_byte(uint8_t dat)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        if (dat & 0x80)
            ec_core_i2c_sda_h();
        else
            ec_core_i2c_sda_l();

        ec_core_i2c_scl_h();
        ec_core_i2c_scl_l();
        dat <<= 1;
    }
    ec_core_i2c_sda_in();
    ec_core_i2c_scl_h();
    i = ec_core_i2c_sda_read();
    ec_core_i2c_scl_l();
    ec_core_i2c_sda_out();
    return (i == 0);
}

static uint8_t ec_core_i2c_read_byte(uint8_t ack)
{
    uint8_t i;
    uint8_t r = 0;
    ec_core_i2c_sda_in();
    ec_core_i2c_delay(2);
    for (i = 0; i < 8; i++)
    {
        ec_core_i2c_scl_h();
        r <<= 1;
        if (ec_core_i2c_sda_read())
        {
            r |= 1;
        }
        ec_core_i2c_scl_l();
    }
    ec_core_i2c_sda_out();
    if (ack)
        ec_core_i2c_sda_l();
    else
        ec_core_i2c_sda_h();
    ec_core_i2c_scl_h();
    ec_core_i2c_delay(2);
    ec_core_i2c_scl_l();
    return r;
}

void ec_core_i2c_write_data(uint8_t dev_addr_7, uint8_t addr, uint8_t *buf, uint8_t size)
{
    int i;
    ec_core_i2c_start();
    if (ec_core_i2c_send_byte(dev_addr_7 << 1) == 0)
    {
        ec_core_i2c_stop();
        return;
    }
    if (ec_core_i2c_send_byte(addr) == 0)
    {
        ec_core_i2c_stop();
        return;
    }
    for (i = 0; i < size; i++)
    {
        if (ec_core_i2c_send_byte(*buf++) == 0)
        {
            ec_core_i2c_stop();
            return;
        }
    }
    ec_core_i2c_stop();
}

void ec_core_i2c_write_one_byte_data(uint8_t dev_addr_7, uint8_t addr, uint8_t buf)
{
    ec_core_i2c_write_data(dev_addr_7, addr, &buf, 1);
}

void ec_core_i2c_write_data_16bit_addr(uint8_t dev_addr_7, uint16_t addr, uint8_t *buf, uint8_t size)
{
    int i;
    ec_core_i2c_start();
    if (ec_core_i2c_send_byte(dev_addr_7 << 1) == 0)
    {
        ec_core_i2c_stop();
        return;
    }
    if (ec_core_i2c_send_byte(addr >> 8) == 0)
    {
        ec_core_i2c_stop();
        return;
    }
    if (ec_core_i2c_send_byte(addr & 0xff) == 0)
    {
        ec_core_i2c_stop();
        return;
    }
    for (i = 0; i < size; i++)
    {
        if (ec_core_i2c_send_byte(*buf++) == 0)
        {
            ec_core_i2c_stop();
            return;
        }
    }
    ec_core_i2c_stop();
}

void ec_core_i2c_write_one_byte_data_16bit_addr(uint8_t dev_addr_7, uint16_t addr, uint8_t buf)
{
    ec_core_i2c_write_data_16bit_addr(dev_addr_7, addr, &buf, 1);
}

void ec_core_i2c_read_data(uint8_t dev_addr_7, uint8_t addr, uint8_t *buf, uint8_t size)
{
    uint8_t i;
    ec_core_i2c_start();
    if (ec_core_i2c_send_byte(dev_addr_7 << 1) == 0)
    {
        ec_core_i2c_stop();
        return;
    }

    if (ec_core_i2c_send_byte(addr) == 0)
    {
        ec_core_i2c_stop();
        return;
    }

    ec_core_i2c_start();
    if (ec_core_i2c_send_byte((dev_addr_7 << 1) + 1) == 0)
    {
        ec_core_i2c_stop();
        return;
    }

    for (i = 0; i < (size - 1); i++)
    {
        *buf++ = ec_core_i2c_read_byte(1);
    }
    *buf++ = ec_core_i2c_read_byte(0);

    ec_core_i2c_stop();
    return;
}

uint8_t ec_core_i2c_read_one_byte_data(uint8_t dev_addr_7, uint8_t addr)
{
    uint8_t buf;
    ec_core_i2c_read_data(dev_addr_7, addr, &buf, 1);
    return buf;
}

void ec_core_i2c_read_data_16bit_addr(uint8_t dev_addr_7, uint16_t addr, uint8_t *buf, uint8_t size)
{
    uint8_t i;
    ec_core_i2c_start();
    if (ec_core_i2c_send_byte(dev_addr_7 << 1) == 0)
    {
        ec_core_i2c_stop();
        return;
    }

    if (ec_core_i2c_send_byte(addr >> 8) == 0)
    {
        ec_core_i2c_stop();
        return;
    }
    if (ec_core_i2c_send_byte(addr & 0xff) == 0)
    {
        ec_core_i2c_stop();
        return;
    }

    ec_core_i2c_start();
    if (ec_core_i2c_send_byte((dev_addr_7 << 1) + 1) == 0)
    {
        ec_core_i2c_stop();
        return;
    }

    for (i = 0; i < (size - 1); i++)
    {
        *buf++ = ec_core_i2c_read_byte(1);
    }
    *buf++ = ec_core_i2c_read_byte(0);

    ec_core_i2c_stop();
    return;
}

uint8_t ec_core_i2c_read_one_byte_data_16bit_addr(uint8_t dev_addr_7, uint16_t addr)
{
    uint8_t buf;
    ec_core_i2c_read_data_16bit_addr(dev_addr_7, addr, &buf, 1);
    return buf;
}
