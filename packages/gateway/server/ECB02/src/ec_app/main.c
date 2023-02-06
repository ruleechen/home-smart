#include "main.h"

int main(void) {
  // 配置系统时钟
  ec_core_sys_clk_set(EC_CORE_SYS_CLK_48M);

  // 初始化蓝牙相关的参数
  rsBleInit();
  rsBlePeripheralInit();

  // 蓝牙内核初始化
  ec_core_init();

  // 使用adc前，需要先初始化，需要放在ec_core_init()之后
  ec_core_adc_init();

  // 初始化串口
  #ifdef VICTOR_DEBUG
    rsUartInit();
  #endif

  // 初始化IO
  rsGpioInit();

  // 初始化软件看门狗，广播超时时间2分钟，蓝牙连接超时时间6分钟
  // ec_core_sw_watchdog_init(EC_CORE_SW_TIMER6, 2, 6);

  // 启用睡眠，串口不能接收数据
  ec_core_sleep_enable();
  // 禁止睡眠，串口可以接收数据
  // ec_core_sleep_disable();

  // 系统主循环
  ec_core_main_loop_run();
}
