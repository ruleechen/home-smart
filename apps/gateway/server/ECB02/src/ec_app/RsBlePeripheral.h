#ifndef __RsBlePeripheral_h
#define __RsBlePeripheral_h

#include <stdbool.h>
#include "ec_core.h"
#include "ec_app_flash.h"
#include "RsServerLib.h"

extern ec_core_gpio_pin_e  RS_GPIO_INPUT_PIN;
extern ec_core_gpio_pin_e  RS_GPIO_OUTPUT_PIN;
extern ec_core_gpio_pin_e  RS_GPIO_READY_PIN;
extern ec_core_adc_ch_e    RS_GPIO_ADC_PIN;
extern ec_core_gpio_pull_e RS_GPIO_INPUT_PULL;

extern ec_core_gpio_level_e rsOutputLevel;
extern ec_core_gpio_level_e rsGetReadyLevel(void);
extern void rsBlePeripheralInit(void);
extern void rsSetInputLevel(ec_core_gpio_level_e level, bool notify);

#endif
