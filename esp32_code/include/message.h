#ifndef MESSAGE_H
#define MESSAGE_H

#include <Arduino.h>

struct Message {
    String addr;
    String cmd;
    String data;
};

Message parseMessage(const String& raw);

String buildMessage(const Message& msg);

String buildMessage(const String& addr, const String& cmd, const String& data);

#endif