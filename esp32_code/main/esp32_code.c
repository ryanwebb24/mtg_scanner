#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define LED_PIN GPIO_NUM_2
#define BUTTON_PIN GPIO_NUM_15
#define TX_PIN GPIO_NUM_16
#define RX_PIN GPIO_NUM_17

void uart_setup() {
  uart_config_t uart_config = {
    .baud_rate = 115200,
    .data_bits = UART_DATA_8_BITS,
    .parity    = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
  };

  uart_driver_install();
  uart_param_config(&uart_config);
  

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
    }
}
