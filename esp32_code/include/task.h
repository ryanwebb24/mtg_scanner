#ifndef TASK_H
#define TASK_H

#include <Arduino.h>

#include <functional>

#include "message.h"
#include "rs485.h"

struct Command {
    String name;
    std::function<void(const String&, const String&)> handler;
};

void processTask(void* param);

#endif