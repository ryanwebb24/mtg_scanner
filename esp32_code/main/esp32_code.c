#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_crc.h"
#include "portmacro.h"
#include "string.h"

#define TX_PIN GPIO_NUM_16
#define RX_PIN GPIO_NUM_17
#define BUFFER_SIZE 1024
#define UART_PORT UART_NUM_1
#define NODE_ID 0x01 // this is hardcoded now but when we do more esps we will need to build the node_id at start when pi will give node_id on init
#define DE_RE_PIN GPIO_NUM_4


void uart_setup(void) {
  uart_config_t uart_config = {
    .baud_rate = 115200,
    .data_bits = UART_DATA_8_BITS,
    .parity    = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
  };


  uart_param_config(UART_PORT, &uart_config);
 
  uart_set_pin(UART_PORT, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

  uart_driver_install(UART_PORT, BUFFER_SIZE, BUFFER_SIZE, 0, NULL, 0);

}

void gpio_setup(void) {
    gpio_config_t config = {
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_ENABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&config);

    gpio_reset_pin(DE_RE_PIN);
    gpio_set_direction(DE_RE_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DE_RE_PIN, 0);  // start in receive mode
}

void send_message(const char *msg) {
  int msg_len = strlen(msg);
  int packet_len = msg_len + 4; // msg + node_id + crc + newline
  uint8_t packet[packet_len];

  packet[0] = NODE_ID;

  memcpy(packet + 1, msg, msg_len);

  uint16_t calc_crc = esp_crc16_le(0, packet, msg_len + 1 );

  packet[msg_len + 1] = calc_crc & 0xFF;
  packet[msg_len + 2] = (calc_crc >> 8) & 0xFF;

  packet[packet_len - 1] = '\n';

  // start transmit
  gpio_set_level(DE_RE_PIN, 1);
  uart_write_bytes(UART_PORT, packet, packet_len);
  uart_wait_tx_done(UART_PORT, 100 / portTICK_PERIOD_MS);
  gpio_set_level(DE_RE_PIN, 0);
}

bool receive_message(uint8_t *buf, int *len) {
    uint8_t tmp[256];
    int msg = uart_read_bytes(UART_PORT, tmp, sizeof(tmp), 100 / portTICK_PERIOD_MS);
    
    if (msg < 4 || tmp[0] != NODE_ID) return false;

    int data_len = msg - 3;
    uint16_t rec_crc = tmp[msg - 3] | (tmp[msg - 2] << 8);
    uint16_t calc_crc = esp_crc16_le(0, tmp, data_len + 1);
    if (rec_crc != calc_crc) return false;

    *len = data_len;
    memcpy(buf, tmp + 1, data_len);
    buf[data_len] = 0;

    return true;
}

typedef enum {
    CMD_UNKNOWN,
    CMD_ERROR,
    CMD_STATUS,
    CMD_RIGHT,
    CMD_LEFT,
    CMD_FORWARD,
} Command;

Command parse_command(const char *msg) {
    if (strncmp(msg, "STATUS", 6) == 0) return CMD_STATUS;
    if (strncmp(msg, "RIGHT", 5) == 0)   return CMD_RIGHT;
    if (strncmp(msg, "LEFT", 4) == 0) return CMD_LEFT; 
    if (strncmp(msg, "FORWARD", 7) == 0) return CMD_FORWARD;
    return CMD_UNKNOWN;
}

void app_main(void) {
    gpio_setup();
    uart_setup();

    uint8_t buf[256];
    int len;
    while (1) {

      if (!receive_message(buf, &len)) {
        send_message("ERROR");
        continue;
      }

      switch (parse_command((char*)buf)) {
        case CMD_STATUS:
          send_message("READY");
          break;

        case CMD_RIGHT:
          //int steps = atoi((char*)buf + 5);
          //move_motor(steps);
          send_message("OK");
          break;

        case CMD_LEFT:
          //stop_motor();
          send_message("OK");
          break;
        
        case CMD_FORWARD:
          send_message("OK");
          break;

        case CMD_UNKNOWN:
        default:
          send_message("ERROR");
          break;
      }
    }
}
