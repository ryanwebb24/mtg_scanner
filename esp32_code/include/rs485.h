#ifndef RS485_H
#define RS485_H

#include <Arduino.h>

#define MAX_MSG_LEN 128
#define CMD_LEN 4

extern QueueHandle_t receiveQueue;
extern QueueHandle_t sendQueue;

void initRS485();
void sendMsg(const String& msg);
void registerDevice(String& addr);

#endif