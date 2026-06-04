#ifndef MESSAGE_H
#define MESSAGE_H

#include <Arduino.h>

struct Message {
    String msg;
    String cmd;
    String data;
    String addr;
};

Message parseMessage(const String& raw);

String buildMessage(const Message& msg);

#endif