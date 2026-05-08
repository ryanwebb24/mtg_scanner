#ifndef UART_H
#define UART_H

#include <stdbool.h>
#include <stdint.h>

#define BUFFER_SIZE 256

uint16_t crc16(const uint8_t* data, int len);
void init_uart(void);
int receive_message(uint8_t* buffer, int buffer_len, int timeout_ms);
void send_message(uint8_t addr, const uint8_t* data, int len);
bool validate_message(uint8_t* buffer, int len);
void uart_task(void* pvParameters);

#endif