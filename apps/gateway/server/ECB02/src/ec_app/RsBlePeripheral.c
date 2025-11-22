#include "RsBlePeripheral.h"

#define DEF_INPUT_LEVEL_ON   EC_CORE_GPIO_LEVEL_L
#define DEF_INPUT_LEVEL_OFF  EC_CORE_GPIO_LEVEL_H

#define DEF_OUTPUT_LEVEL_ON  EC_CORE_GPIO_LEVEL_H
#define DEF_OUTPUT_LEVEL_OFF EC_CORE_GPIO_LEVEL_L

#define DEF_READY_LEVEL_ON   EC_CORE_GPIO_LEVEL_L
#define DEF_READY_LEVEL_OFF  EC_CORE_GPIO_LEVEL_H

ec_core_gpio_pin_e  RS_GPIO_INPUT_PIN  = EC_CORE_GPIO_P3;     // switch on/off input pin
ec_core_gpio_pin_e  RS_GPIO_OUTPUT_PIN = EC_CORE_GPIO_P4;     // alarm on/off output pin
ec_core_gpio_pin_e  RS_GPIO_READY_PIN  = EC_CORE_GPIO_P5;     // signal for ble ready
ec_core_adc_ch_e    RS_GPIO_ADC_PIN    = EC_CORE_ADC_CH4_P10; // battery detection
ec_core_gpio_pull_e RS_GPIO_INPUT_PULL = DEF_INPUT_LEVEL_OFF == EC_CORE_GPIO_LEVEL_H ? EC_CORE_GPIO_PULL_UP_S : EC_CORE_GPIO_PULL_DOWN;

ec_core_gpio_level_e rsInputLevel; // default value will be initialized on startup
ec_core_gpio_level_e rsOutputLevel = DEF_OUTPUT_LEVEL_OFF;
ec_core_gpio_level_e rsGetReadyLevel(void) {
  return rsAuthenticated ? DEF_READY_LEVEL_ON : DEF_READY_LEVEL_OFF;
}
static void updateReadyState() {
  ec_core_gpio_write(RS_GPIO_READY_PIN, rsGetReadyLevel());
}

static void notifyStates(RsServerCommandType command) {
  uint16_t value, voltage; // 寄存器值和电压值
  ec_core_adc_get(RS_GPIO_ADC_PIN, EC_CORE_ADC_RANGE_3200MV, EC_CORE_ADC_CALIBRATION_ENABLED, &value, &voltage);
  rsEmitStates(
    command,                                                                                    // command
    (uint8_t)((voltage & 0xFF00) >> 8),                                                         // data1
    (uint8_t)(voltage & 0x00FF),                                                                // data2
    rsOtaEnabled                         ? RS_SERVER_STATE_OTA_ON    : RS_SERVER_STATE_UNKNOWN, // data3
    rsInputLevel  == DEF_INPUT_LEVEL_ON  ? RS_SERVER_STATE_INPUT_ON  : RS_SERVER_STATE_UNKNOWN, // data4
    rsOutputLevel == DEF_OUTPUT_LEVEL_ON ? RS_SERVER_STATE_OUTPUT_ON : RS_SERVER_STATE_UNKNOWN, // data5
    RS_SERVER_STATE_UNKNOWN,                                                                    // data6
    RS_SERVER_STATE_UNKNOWN,                                                                    // data7
    RS_SERVER_STATE_UNKNOWN                                                                     // data8
  );
}

static void disconnect(void) {
  ec_core_ble_disconnect();
}

static void activate(void) {
  ec_core_sw_watchdog_feed();
  rsAuthenticated = false;
  updateReadyState();
  ec_core_sw_timer_start(EC_CORE_SW_TIMER1, 10 * 1000, disconnect);
}

static void teardown(void) {
  rsAuthenticated = false;
  updateReadyState();
  ec_core_sw_timer_stop(EC_CORE_SW_TIMER1);
}

static void dataEmitHandler(uint8_t* data, size_t length) {
  ec_core_ble_send(data, length);
  #ifdef VICTOR_DEBUG
    ec_core_uart0_printf(" > ble emit [%s]\r\n", data);
  #endif
}

static void commandHandler(uint8_t* payload, size_t length) {
  if (
    payload == NULL ||
    rsAuthenticated == false
  ) {
    ec_core_ble_disconnect();
    return;
  }
  RsServerCommandType command = (RsServerCommandType)payload[RS_PROTOCOL_INDEX_COMMAND];
  switch (command) {
    case RS_SERVER_COMMAND_AUTHENTICATE: {
      ec_core_sw_timer_stop(EC_CORE_SW_TIMER1);
      notifyStates(command);
      updateReadyState();
      break;
    }

    case RS_SERVER_COMMAND_HEARTBEAT: {
      ec_core_sw_watchdog_feed();
      notifyStates(command);
      break;
    }

    case RS_SERVER_COMMAND_GET_STATES: {
      notifyStates(command);
      break;
    }

    case RS_SERVER_COMMAND_SET_STATES: {
      // ota
      uint8_t otaEnabled = payload[RS_PROTOCOL_INDEX_DATA3] == RS_SERVER_STATE_OTA_ON;
      if (rsOtaEnabled != otaEnabled) {
        rsOtaEnabled = otaEnabled;      // 修改内存
        ec_app_flash_sys_param_write(); // 保存到flash
        ec_core_delay_ms(20);           // 延迟20ms
        ec_core_sys_soft_reset();       // 系统复位
      }
      // output
      ec_core_gpio_level_e outputLevel = payload[RS_PROTOCOL_INDEX_DATA5] == RS_SERVER_STATE_OUTPUT_ON ? DEF_OUTPUT_LEVEL_ON : DEF_OUTPUT_LEVEL_OFF;
      if (rsOutputLevel != outputLevel) {
        rsOutputLevel = outputLevel;
        ec_core_gpio_write(RS_GPIO_OUTPUT_PIN, rsOutputLevel);
      }
      // notify
      notifyStates(command);
      break;
    }

    case RS_SERVER_COMMAND_RESET: {
      ec_core_sys_soft_reset();
      break;
    }

    default:
    case RS_SERVER_COMMAND_UNKNOWN: {
      // ignore
      break;
    }
  }
}

static void onPeripheralConnect(void) { //蓝牙连接回调
  activate();
  #ifdef VICTOR_DEBUG
    ec_core_uart0_printf("ble peripheral connect\r\n");
  #endif
}
static void onPeripheralDisconnect(void) { //蓝牙断开回调
  teardown();
  #ifdef VICTOR_DEBUG
    ec_core_uart0_printf("ble peripheral disconnect\r\n");
  #endif
}
static void onPeripheralReceive(uint8_t* data, uint8_t length) { //蓝牙数据接收回调
  ec_core_uart_send(EC_CORE_UART0, data, length); // 蓝牙数据转发到串口
  rsHandleMessage(data, length);
}

void rsBlePeripheralInit(void) {
  // 从flash读取并应用系统参数
  ec_app_flash_sys_param_read();
  ec_core_ble_peripheral_set_ota_en(rsOtaEnabled);
  // 初始化Server
  rsServerInit(RS_SERVER_MODEL_DOOR_SENSOR, dataEmitHandler, commandHandler);
  // 注册蓝牙事件回调
  ec_core_ble_peripheral_set_event_cb(EC_CORE_BLE_PERIPHERAL_EVENT_CONNECT, onPeripheralConnect);       // 注册蓝牙连接的回调
  ec_core_ble_peripheral_set_event_cb(EC_CORE_BLE_PERIPHERAL_EVENT_DISCONNECT, onPeripheralDisconnect); // 注册蓝牙断开的回调
  ec_core_ble_peripheral_set_receive_cb(onPeripheralReceive);                                           // 注册蓝牙数据接收的回调
}

static bool inputThrottled = false;
static ec_core_gpio_level_e cachedLevel;
static bool cachedNotify = false;

static void stopThrottling() {
  ec_core_sw_timer_stop(EC_CORE_SW_TIMER2);
  inputThrottled = false;
  rsSetInputLevel(cachedLevel, cachedNotify);
}

void rsSetInputLevel(ec_core_gpio_level_e level, bool notify) {
  if (inputThrottled) {
    cachedLevel = level;
    cachedNotify = notify;
    return;
  }
  if (rsInputLevel == level) {
    return;
  }
  rsInputLevel = level;
  if (notify) {
    inputThrottled = true;
    cachedLevel = rsInputLevel;
    notifyStates(RS_SERVER_COMMAND_NOTIFY_STATES);
    ec_core_sw_timer_start(EC_CORE_SW_TIMER2, 100, stopThrottling);
  }
}
