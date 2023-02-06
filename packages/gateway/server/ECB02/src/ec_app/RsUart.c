#include "RsUart.h"

static ec_core_gpio_pin_e RS_GPIO_UART_TX = EC_CORE_GPIO_P1; // UART0 tx pin
static ec_core_gpio_pin_e RS_GPIO_UART_RX = EC_CORE_GPIO_P2; // UART0 rx pin

#define EC_APP_UART0_TX_BUF_SIZE 128                  // 串口0发送缓冲区大小，可以根据需要调整
#define EC_APP_UART0_RX_BUF_SIZE 128                  // 串口0接收缓冲区大小，可以根据需要调整
uint8_t uart0_tx_buf[EC_APP_UART0_TX_BUF_SIZE] = {0}; // 串口0发送缓冲区
uint8_t uart0_rx_buf[EC_APP_UART0_RX_BUF_SIZE] = {0}; // 串口0接收缓冲区

static void uartRx(uint8_t* buffer, uint16_t length) {
  ec_core_uart_send(EC_CORE_UART0, buffer, length); // ECHO 回显
  rsHandleMessage(buffer, length);
}

void rsUartInit(void) {
  // init uart0
  ec_core_uart_init(EC_CORE_UART0, 9600, EC_CORE_UART_PARITY_NONE, RS_GPIO_UART_TX, RS_GPIO_UART_RX, uart0_tx_buf, EC_APP_UART0_TX_BUF_SIZE, uart0_rx_buf, EC_APP_UART0_RX_BUF_SIZE, uartRx);
  // print core version
  uint8_t ver[3] = {0};
  ec_core_ver(ver);
  ec_core_uart0_printf("ECB02 SDK %d.%d.%d\r\n", ver[0], ver[1], ver[2]);
}
