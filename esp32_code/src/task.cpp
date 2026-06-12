#include "task.h"

void handleForward(const String& data) {
    Serial.println("forward");
}
void handleLeft(const String& data) {
    Serial.println("left");
}
void handleRight(const String& data) {
    Serial.println("right");
}

Command commands[] = {
    {"forward", handleForward},
    {"left", handleLeft},
    {"right", handleRight},
};

void processTask(void* param) {
    String raw;
    while (true) {
        char raw[MAX_MSG_LEN];
        if (xQueueReceive(receiveQueue, raw, portMAX_DELAY)) {
            Message msg = parseMessage(String(raw));
            for (auto& c : commands) {
                if (c.name == msg.cmd) {
                    c.handler(msg.data);
                    break;
                }
            }
        }
    }
}