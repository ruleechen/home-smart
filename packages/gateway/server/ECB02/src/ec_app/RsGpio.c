#include "RsGpio.h"

static void inputRising(void) {
  rsSetInputLevel(EC_CORE_GPIO_LEVEL_H, true);
}

static void inputFalling(void) {
  rsSetInputLevel(EC_CORE_GPIO_LEVEL_L, true);
}

void rsGpioInit(void) {
  // input
  ec_core_gpio_in_init(RS_GPIO_INPUT, EC_CORE_GPIO_PULL_UP_S);         // 初始化输入，低电平有效
  ec_core_gpio_int_register(RS_GPIO_INPUT, inputRising, inputFalling); // 中断使能
  rsSetInputLevel(ec_core_gpio_read(RS_GPIO_INPUT), false);

  // output
  ec_core_gpio_out_init(RS_GPIO_OUTPUT, rsOutputLevel);
  ec_core_gpio_out_init(RS_GPIO_READY,  rsGetReadyLevel());
}
