#pragma once

#include "logger/logger.h"

namespace bmc {

class NetworkInterface {
public:
    explicit NetworkInterface(Logger& logger)
        : logger_(logger) {}

    virtual ~NetworkInterface() = default;

    virtual bool connect() = 0;
    virtual bool isConnected() const = 0;
    virtual void disconnect() = 0;

protected:
    Logger& logger_;
};

}