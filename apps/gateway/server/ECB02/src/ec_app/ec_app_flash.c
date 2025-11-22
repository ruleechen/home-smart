#include "ec_app_flash.h"

// 1: 允许通过蓝牙无线升级程序
// 0: 禁止无线升级程序，需要重新上电，拉高BOOT引脚才能进入下载模式
uint8_t rsOtaEnabled = 1; // default enabled

//在flash中，可靠存储系统参数
//使用了双备份和CRC校验，写和读采用两块扇区

//最后三块扇区的地址
#define EC_APP_FLASH_ADDR_SELECT 0x3d000 // 判断应该操作哪块扇区进行数据的读和写
#define EC_APP_FLASH_ADDR_DATA1 0x3e000  //数据1
#define EC_APP_FLASH_ADDR_DATA2 0x3f000  //数据2

#define EC_APP_FLASH_BUF_SIZE 200 //假设有200个字节的系统参数需要保存

static uint8_t ec_app_flash_sector_select = 0;                    //扇区操作标志位
static uint8_t ec_app_flash_buf[EC_APP_FLASH_BUF_SIZE + 2] = {0}; // 数据+2字节CRC

uint16_t ec_app_flash_crc(uint8_t *data, uint16_t len) // CRC16计算
{
  uint8_t lsb;
  uint16_t i, j;
  uint16_t h = 0xffff;
  for (i = 0; i < len; i++) {
    h ^= *data;
    for (j = 0; j < 8; j++) {
      lsb = h & 0x0001;
      h >>= 1;
      if (lsb == 1) {
        h ^= 0xECB2;
      }
    }
    data++;
  }
  h ^= 0xffff;
  return h;
}

static void ec_app_flash_sector_select_set(uint8_t n) //设置标志位
{
  ec_core_flash_erase_sector(EC_APP_FLASH_ADDR_SELECT); //擦除
  ec_core_flash_write(EC_APP_FLASH_ADDR_SELECT, &n, 1); //写入
}
static uint8_t ec_app_flash_sector_select_get(void) //读取标志位
{
  uint8_t buff;
  ec_core_flash_read(EC_APP_FLASH_ADDR_SELECT, &buff, 1); //读取
  return buff;
}

static void ec_app_flash_safe_write(void) //系统参数保存到flash
{
  uint16_t crc = ec_app_flash_crc(ec_app_flash_buf, EC_APP_FLASH_BUF_SIZE); //计算CRC
  ec_app_flash_buf[EC_APP_FLASH_BUF_SIZE] = crc >> 8;                       //保存CRC到缓冲区
  ec_app_flash_buf[EC_APP_FLASH_BUF_SIZE + 1] = crc & 0xff;                 //保存CRC到缓冲区

  ec_app_flash_sector_select = ec_app_flash_sector_select_get(); //读取标志位
  if (ec_app_flash_sector_select == 0) {
    //操作第二块扇区
    ec_core_flash_erase_sector(EC_APP_FLASH_ADDR_DATA2);                                       //擦除
    ec_core_flash_write(EC_APP_FLASH_ADDR_DATA2, ec_app_flash_buf, EC_APP_FLASH_BUF_SIZE + 2); //写入
    ec_app_flash_sector_select = 1;                                                            //修改标志位
    ec_app_flash_sector_select_set(ec_app_flash_sector_select);                                //保存标志位到flash
  } else {
    //操作第一块扇区
    ec_core_flash_erase_sector(EC_APP_FLASH_ADDR_DATA1);                                       //擦除
    ec_core_flash_write(EC_APP_FLASH_ADDR_DATA1, ec_app_flash_buf, EC_APP_FLASH_BUF_SIZE + 2); //写入
    ec_app_flash_sector_select = 0;                                                            //修改标志位
    ec_app_flash_sector_select_set(ec_app_flash_sector_select);                                //保存标志位到flash
  }
}
static uint8_t ec_app_flash_safe_read() //从flash中读取系统参数
{
  uint16_t crc1, crc2;
  ec_app_flash_sector_select = ec_app_flash_sector_select_get(); //读取标志位
  if (ec_app_flash_sector_select == 0) {
    //操作第一块扇区
    ec_core_flash_read(EC_APP_FLASH_ADDR_DATA1, ec_app_flash_buf, EC_APP_FLASH_BUF_SIZE + 2); //读取flash
  } else {
    //操作第二块扇区
    ec_core_flash_read(EC_APP_FLASH_ADDR_DATA2, ec_app_flash_buf, EC_APP_FLASH_BUF_SIZE + 2); //读取flash
  }
  crc1 = ec_app_flash_crc(ec_app_flash_buf, EC_APP_FLASH_BUF_SIZE);                                   //计算CRC
  crc2 = ec_app_flash_buf[EC_APP_FLASH_BUF_SIZE] * 256 + ec_app_flash_buf[EC_APP_FLASH_BUF_SIZE + 1]; //读取存储的CRC
  if (crc1 != crc2) {
    // CRC出错，数据不可用
    return 1;
  }

  //读取成功
  return 0;
}

void ec_app_flash_sys_param_write(void) {
  ec_core_delay_ms(20); //延迟20ms 操作flash之前先延迟一会 避免和串口冲突

  //保存参数到缓存
  ec_app_flash_buf[0] = rsOtaEnabled;

  //保存参数到flash
  ec_app_flash_safe_write();
}

void ec_app_flash_sys_param_read(void) {
  uint8_t result = 0;

  //读取系统参数
  for (uint8_t i = 0; i < 3; i++) {
    if (ec_app_flash_safe_read() == 0) {
      //读取成功
      result = 1;
      break;
    } else {
      //读取失败
      ec_core_delay_ms(100); //延迟100ms
    }
  }

  if (result == 0) //读取失败
  {
    ec_app_flash_sys_param_write(); //默认参数 保存到flash
    return;
  }

  // 提取参数到系统
  rsOtaEnabled = ec_app_flash_buf[0];
}
