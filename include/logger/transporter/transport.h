#pragma once

#include "logger/queue.h"

class LogTransport {
public:
    virtual ~LogTransport() = default;

    virtual bool available() const = 0;
    virtual bool send(const LogEntry& entry) = 0;
};