/******************************************************************************
 *  Copyright © 2019 Shenzhen ECIOT Technology Co., Ltd All Rights Reserved
 *-----------------------------------------------------------------------------
 * @file        ec_app_power_test.c
 * @brief       set power demo
 * @author      mo@eciot.cn (qq:2201920828,v:eciot_mo)
 * @date        2022-02-13
 * @version     1.0
 ******************************************************************************/

#include "main.h"

#ifdef EC_APP_POWER_TEST

#include "ec_core.h"

int main(void)
{
    //配置蓝牙芯片的发射功率 范围-20dbm ~ 10dbm

    ec_core_sys_clk_set(EC_CORE_SYS_CLK_48M); //系统时钟初始化

    ec_core_ble_set_power(EC_CORE_BLE_POWER_N20DBM); //最小功率
    // ec_core_ble_set_power(EC_CORE_BLE_POWER_3DBM); //默认功率
    // ec_core_ble_set_power(EC_CORE_BLE_POWER_10DBM); //最大功率

    ec_core_init();          //蓝牙内核初始化
    ec_core_sleep_disable(); //禁止睡眠，串口可以接收数据
    ec_core_main_loop_run(); //系统主循环
}

#endif
