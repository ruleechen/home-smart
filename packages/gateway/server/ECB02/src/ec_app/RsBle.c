#include "RsBle.h"

void rsBleInit(void) {
  // 默认从机，PERIPHERAL
  ec_core_set_role(EC_CORE_BLE_ROLE_PERIPHERAL);

  // 配置发射功率，默认3dbm
  ec_core_ble_set_power(EC_CORE_BLE_POWER_5DBM);

  // 配置UUID - 16bit uuid
  ec_core_ble_set_suuid("AF00"); // 默认为FFF0
  ec_core_ble_set_ruuid("AF01"); // 默认为FFF1
  ec_core_ble_set_wuuid("AF02"); // 默认为FFF2

  // 根据MAC地址修改蓝牙名字
  uint8_t mac[6] = {0};
  ec_core_ble_get_mac(mac); // 获取MAC地址
  char peripheralName[16] = {0};
  sprintf(peripheralName, "RS031_%02X%02X%02X", mac[3], mac[4], mac[5]);
  ec_core_ble_peripheral_set_name((uint8_t*)peripheralName, 12); // strlen("BT_123")

  // 厂商自定义数据 - "RuleeSmart-V01"
  char manufacturerData[RS_PROTOCOL_MANUFACTURER_DATA_LENGTH + 1] = {0};
  sprintf(manufacturerData, RS_PROTOCOL_MANUFACTURER_DATA);
  ec_core_ble_peripheral_set_manufacturer_data((uint8_t*)manufacturerData, RS_PROTOCOL_MANUFACTURER_DATA_LENGTH);

  // 设置广播间隔
  // 3200*0.625=2s
  ec_core_ble_peripheral_set_adv_int(3200);

  // 低功耗，延迟大（理论平均电流50uA）
  // 160*1.25=200ms, 180*1.25=225ms, 4, 600*10=6s
  ec_core_ble_peripheral_set_connect_param(160, 180, 4, 600);
  // 折中方案（理论平均电流100uA）
  // 80*1.25=100ms, 120*1.25=150ms, 4, 300*10=3s
  // ec_core_ble_peripheral_set_connect_param(80, 120, 4, 300);
}
