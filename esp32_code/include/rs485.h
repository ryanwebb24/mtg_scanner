#ifndef RS485_H
#define RS485_h

#include <Arduino.h>

extern QueueHandle_t receiveQueue;
extern QueueHandle_t sendQueue;

void initRS485();

void sendMsgTask(void* param);

void sendMsg(const String& msg);
void registerDevice(String& addr);

#endif