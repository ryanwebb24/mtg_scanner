#include "rs485.h"

#include <Arduino.h>
#include <WiFi.h>

#include "message.h"

#define DE_PIN 4
#define RS485_SERIAL Serial1

QueueHandle_t receiveQueue;
QueueHandle_t sendQueue;

void sendMsgTask(void* param) {
    String msg;
    while (true) {
        if (xQueueReceive(sendQueue, &msg, portMAX_DELAY)) {
            digitalWrite(DE_PIN, HIGH);
            RS485_SERIAL.print(msg);
            RS485_SERIAL.flush();
            digitalWrite(DE_PIN, LOW);
        }
    }
}

void sendMsg(const String& msg) {
    xQueueSend(sendQueue, &msg, portMAX_DELAY);
}

void receiveMsgTask(void* param) {
    String msg = "";
    while (true) {
        while (RS485_SERIAL.available()) {
            char c = RS485_SERIAL.read();
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
    pinMode(DE_PIN, OUTPUT);
    digitalWrite(DE_PIN, LOW);
    RS485_SERIAL.begin(115200);
    receiveQueue = xQueueCreate(10, sizeof(String));
    sendQueue = xQueueCreate(10, sizeof(String));
    xTaskCreate(sendMsgTask, "send", 4096, NULL, 2, NULL);
    xTaskCreate(receiveMsgTask, "receive", 4096, NULL, 2, NULL);
}

void registerDevice(String& addr) {
    String mac = WiFi.macAddress();
    sendMsg("REGISTER:" + mac + "\n");
    String raw;
    if (xQueueReceive(receiveQueue, &raw, pdMS_TO_TICKS(5000))) {
        Message msg = parseMessage(raw);
        if (msg.cmd == "ADDR") {
            addr = msg.data;
        }
    }
}