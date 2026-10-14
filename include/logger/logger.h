#pragma once

#include "queue.h"
#include "logger/transporter/serial.h"
#include "logger/transporter/udp.h"

#include <stdint.h>

namespace bmc {
    class NetworkManager;
}

class Logger {
public:
    explicit Logger(const char* component)
        : component_(component) {}

    static void begin(const bmc::NetworkManager& manager);

    void info(const char* message);
    void error(const char* message);

private:
    const char* component_;
    
    static LogQueue queue_;
    static UdpLogTransport udpTransport_;
    static SerialLogTransport serialTransport_;

    void log(LogLevel level, const char* message);

    static void loggingTask(void* parameter);
};