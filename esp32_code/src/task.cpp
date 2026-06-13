#include "task.h"

#include "stepper.h"

void handleStepper(const String& cmd, const String& data) {
    char c[CMD_LEN];
    cmd.toCharArray(c, CMD_LEN);
    xQueueSend(stepperQueue, c, portMAX_DELAY);
}

Command commands[] = {
    {"FWD", handleStepper},
    {"LFT", handleStepper},
    {"RGT", handleStepper},
};

void processTask(void* param) {
    char raw[MAX_MSG_LEN];
    while (true) {
        if (xQueueReceive(receiveQueue, raw, portMAX_DELAY)) {
            Message msg = parseMessage(String(raw));
            for (auto& c : commands) {
                if (c.name == msg.cmd) {
                    c.handler(msg.cmd, msg.data);
                    break;
                }
            }
        }
    }
}