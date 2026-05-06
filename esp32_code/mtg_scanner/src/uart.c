#include "driver/uart.h"

#include "freertos/queue.h"

#define BUFFER_SIZE 1048

uint16_t crc16(const uint8_t* data, int len) {
    uint16_t crc = 0xFFFF;
    for (int i = 0; i < len; i++) {
        crc ^= data[i];                // XOR byte into low bits of crc
        for (int j = 0; j < 8; j++) {  // process each bit
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;  // 0xA001 is 0x8005 reversed
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

void init_uart() {
    const int uart_buffer_size = 2048;
    const int tx_pin = 17;
    const int rx_pin = 16;
    const int rts_pin = 4;
    QueueHandle_t uart_queue;

    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, uart_buffer_size, uart_buffer_size, 10, &uart_queue, 0));

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS,
        .rx_flow_ctrl_thresh = 122,
    };
    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &uart_config));

    // Set UART pins(TX: IO4, RX: IO5, RTS: IO18, CTS: IO19, DTR: UNUSED, DSR: UNUSED)
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, tx_pin, rx_pin, rts_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    ESP_ERROR_CHECK(uart_set_mode(UART_NUM_2, UART_MODE_RS485_HALF_DUPLEX));
}

int receive_message(uint8_t* data) {
    int len = uart_read_bytes(UART_NUM_2, data);
}

void send_message(uint8_t addr, const uint8_t* data, int len) {
    uint8_t temp_buff[256];

    temp_buff[0] = addr;
    for (int i; i < len; i++) {
        temp_buff[i + 1] = data[i];
    }
    temp_buff[len + 1] = "\n";

    uint16_t crc = crc16(temp_buff, len + 1);
    temp_buff[len + 2] = crc & 0xFF;
    temp_buff[len + 3] = (crc >> 8) & 0xFF;

    uart_write_bytes(UART_NUM_2, temp_buff, len + 4);
}