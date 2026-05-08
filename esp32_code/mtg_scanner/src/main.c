#include "esp_log.h"
#include "uart.h"

void app_main() {
    ESP_LOGI("MAIN", "booted");
    init_uart();
    ESP_LOGI("MAIN", "uart init done");
    xTaskCreate(uart_task, "uart_task", 2048, NULL, 5, NULL);
}