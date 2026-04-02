#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"

#define LED_PIN GPIO_NUM_2
#define BUTTON_PIN GPIO_NUM_15
#define TX_PIN GPIO_NUM_16
#define RX_PIN GPIO_NUM_17
#define BUFFER_SIZE 1024
#define UART_PORT UART_NUM_1

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

void app_main(void) {
    gpio_config_t config = {
      .pin_bit_mask = (1ULL << BUTTON_PIN),
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_ENABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&config);

    uart_setup();

    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    int delay = 500;
    while (1) {
        if (gpio_get_level(BUTTON_PIN) == 1) {
          delay = 100;
        } else {
          delay = 500;
        }
        gpio_set_level(LED_PIN, 1);
        vTaskDelay(delay / portTICK_PERIOD_MS);
        gpio_set_level(LED_PIN, 0);
        vTaskDelay(delay / portTICK_PERIOD_MS);
        uart_write_bytes(UART_PORT, "hello\n", 6);
        uint8_t data[BUFFER_SIZE];
        
        int len = uart_read_bytes(UART_PORT, data, BUFFER_SIZE, 20 / portTICK_PERIOD_MS);
        if (len > 0) {
          data[len] = 0; // null terminate so you can print as string
          printf("received: %s", data);
        }
    }
}
