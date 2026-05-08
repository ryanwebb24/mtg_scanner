#include "esp_log.h"
#include "uart.h"

void app_main() {
    ESP_LOGI("MAIN", "booted");
    init_uart();
    ESP_LOGI("MAIN", "uart init done");
    // xTaskCreate(uart_task, "uart_task", 2048, NULL, 5, NULL);

    uint8_t msg[] = "hello";
    uint8_t buf[BUFFER_SIZE];

    // send a message to address 0x01
    send_message(0x01, msg, 5);

    // try to receive it
    int len = receive_message(buf, BUFFER_SIZE, 1000);
    ESP_LOGI("TEST", "receive returned: %d bytes", len);
    for (int i = 0; i < len; i++) {
        ESP_LOGI("TEST", "buf[%d] = 0x%02X", i, buf[i]);
    }

    if (len < 0) {
        ESP_LOGI("TEST", "timeout, nothing received");
    } else if (!validate_message(buf, len)) {
        ESP_LOGI("TEST", "CRC failed");
    } else {
        ESP_LOGI("TEST", "got message, addr: 0x%02X, len: %d", buf[0], len);
    }
}