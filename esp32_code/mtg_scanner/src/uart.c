#include "driver/uart.h"

#include "freertos/queue.h"
#include "uart.h"

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

void init_uart(void) {
    const int uart_buffer_size = 2048;
    const int tx_pin = 17;
    const int rx_pin = 16;
    QueueHandle_t uart_queue;

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, uart_buffer_size, uart_buffer_size, 10, &uart_queue, 0));
    ESP_ERROR_CHECK(uart_set_mode(UART_NUM_2, UART_MODE_UART));
    // ESP_ERROR_CHECK(uart_set_mode(UART_NUM_2, UART_MODE_RS485_HALF_DUPLEX)); // RS485
}

int receive_message(uint8_t* buffer, int buffer_len, int timeout_ms) {
    int i = 0;
    uint8_t byte;

    while (i < buffer_len - 1) {
        int ret = uart_read_bytes(UART_NUM_2, &byte, 1, pdMS_TO_TICKS(timeout_ms));

        if (ret <= 0) {
            return -1;  // timeout, no data
        }

        buffer[i++] = byte;

        if (byte == '\n') {
            // read 2 CRC bytes
            uart_read_bytes(UART_NUM_2, &buffer[i++], 1, pdMS_TO_TICKS(timeout_ms));
            uart_read_bytes(UART_NUM_2, &buffer[i++], 1, pdMS_TO_TICKS(timeout_ms));
            break;
        }
    }

    return i;  // how many bytes we got
}

void send_message(uint8_t addr, const uint8_t* data, int len) {
    uint8_t temp_buff[BUFFER_SIZE];

    temp_buff[0] = addr;
    for (int i = 0; i < len; i++) {
        temp_buff[i + 1] = data[i];
    }
    temp_buff[len + 1] = '\n';
    uint16_t crc = crc16(temp_buff, len + 2);
    temp_buff[len + 2] = crc & 0xFF;
    temp_buff[len + 3] = (crc >> 8) & 0xFF;
    uart_write_bytes(UART_NUM_2, temp_buff, len + 4);
}

bool validate_message(uint8_t* buffer, int len) {
    if (len < 4) return false;

    uint16_t received_crc = buffer[len - 2] | (buffer[len - 1] << 8);

    uint16_t computed_crc = crc16(buffer, len - 2);

    return received_crc == computed_crc;
}

void uart_task(void* pvParameters) {
    uint8_t buf[BUFFER_SIZE];

    while (1) {
        int len = receive_message(buf, BUFFER_SIZE, 100);

        if (len < 0) {
            continue;
        }

        if (!validate_message(buf, len)) {
            continue;
        }

        // uint8_t addr = buf[0];
        //  buf[1] to buf[len-3] is your data

        // do something based on what came in
    }
}