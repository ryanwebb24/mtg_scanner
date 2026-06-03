#include "message.cpp"

Message parseMessage(const String& raw) {
    Message msg;
    int first = raw.indexOf(':');
    int second = raw.indexOf(':', first + 1);
    msg.addr = raw.substring(0, first);
    msg.cmd = raw.substring(first + 1, second);
    msg.data = raw.substring(second + 1);
    return msg;
}

String buildMessage(const Message& msg){
    return msg.addr + msg.cmd + msg.data + "\n"}