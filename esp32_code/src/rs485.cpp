#include "rs485.h"

#include <Arduino.h>
#include <WiFi.h>

#include "message.h"

#define DE_PIN 4
#define RS485_SERIAL Serial2
#define PI_ADRESS "00"

QueueHandle_t receiveQueue;
QueueHandle_t sendQueue;

void sendMsgTask(void* param) {
    char msg[MAX_MSG_LEN];
    while (true) {
        if (xQueueReceive(sendQueue, msg, portMAX_DELAY)) {
            digitalWrite(DE_PIN, HIGH);
            RS485_SERIAL.print(msg);
            RS485_SERIAL.flush();
            digitalWrite(DE_PIN, LOW);
        }
    }
}

void sendMsg(const String& msg) {
    char buf[MAX_MSG_LEN];
    msg.toCharArray(buf, MAX_MSG_LEN);
    xQueueSend(sendQueue, buf, portMAX_DELAY);
}

void receiveMsgTask(void* param) {
    char msg[MAX_MSG_LEN] = {0};
    int idx = 0;
    while (true) {
        while (RS485_SERIAL.available()) {
            char c = RS485_SERIAL.read();
            if (c == '\n') {
                msg[idx] = '\0';
                xQueueSend(receiveQueue, msg, portMAX_DELAY);
                idx = 0;
                memset(msg, 0, MAX_MSG_LEN);
            } else if (idx < MAX_MSG_LEN - 1) {
                msg[idx++] = c;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void initRS485() {
    Serial.begin(115200);
    Serial.println("initRS485 start");
    pinMode(DE_PIN, OUTPUT);
    digitalWrite(DE_PIN, LOW);
    RS485_SERIAL.begin(115200, SERIAL_8N1, 16, 17);
    receiveQueue = xQueueCreate(10, MAX_MSG_LEN);
    sendQueue = xQueueCreate(10, MAX_MSG_LEN);
    xTaskCreate(sendMsgTask, "send", 4096, NULL, 2, NULL);
    xTaskCreate(receiveMsgTask, "receive", 4096, NULL, 2, NULL);
    Serial.println("initRS485 done");
}

void registerDevice(String& addr) {
    String mac = WiFi.macAddress();
    while (addr == "") {
        sendMsg(buildMessage("00", "REG", mac));
        char raw[MAX_MSG_LEN];
        if (xQueueReceive(receiveQueue, raw, pdMS_TO_TICKS(5000))) {
            Message msg = parseMessage(String(raw));
            if (msg.cmd == "ADR" && msg.data != "") {
                addr = msg.data;
                Serial.println("assigned addr: " + addr);
            }
        } else {
            Serial.println("timeout, retrying...");
        }
    }
}