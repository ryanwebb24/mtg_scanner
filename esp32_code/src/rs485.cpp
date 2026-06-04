#include "rs485.h"

#include <ArduinoRS485.h>

extern QueueHandle_t receiveQueue;
extern QueueHandle_t sendQueue;

void sendMsgTask(void* param) {
    String msg;
    while (true) {
        if (xQueueReceive(sendQueue, &msg, portMAX_DELAY)) {
            RS485.beginTransmission();
            RS485.print(msg);
            RS485.endTransmission();
            RS485.receive();
        }
    }
}

void sendMsg(const String& msg) {
    xQueueSend(sendQueue, &msg, portMAX_DELAY);
}

void receiveMsgTask(void* param) {
    String msg = "";
    while (true) {
        while (RS485.available()) {
            char c = RS485.read();
            if (c == '\n') {
                xQueueSend(receiveQueue, &msg, portMAX_DELAY);
                msg = "";
            } else {
                msg += c;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void initRS485() {
    receiveQueue = xQueueCreate(10, sizeof(String));
    sendQueue = xQueueCreate(10, sizeof(String));
    RS485.begin(115200);
    RS485.receive();
    xTaskCreate(sendMsgTask, "send", 4096, NULL, 2, NULL);
}

void registerDevice(String& addr) {
    String mac = WiFi.macAddress();
    sendMsg("REGISTER:" + mac + "\n");

    String raw;
    if (xQueueReceive(receiveQueue, &raw, pdMS_TO_TICKS(5000))) {
        Message m = parseMessage(raw);
        if (m.cmd == "ADDR") {
            addr = m.data;
        }
    }
}