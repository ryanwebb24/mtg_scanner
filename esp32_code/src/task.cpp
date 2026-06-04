#include "task.h"

#include <Arduino.h>

#include "message.h"
#include "rs485.h"

void processTask(void* param) {
    String raw;
    while (true) {
        if (xQueueReceive(receiveQueue, &raw, portMAX_DELAY)) {
            Message msg = parseMessage(raw);

            Serial.println("cmd: " + msg.cmd + " data: " + msg.data);
            if (msg.cmd == "forward") {
            } else if (msg.cmd == "left") {
            } else if (msg.cmd == "right") {
            }
        }
    }
}