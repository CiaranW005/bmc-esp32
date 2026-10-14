#pragma once

#include "logger/transporter/transport.h"

class SerialLogTransport : public LogTransport {
public:
    bool available() const override;
    bool send(const LogEntry& entry) override;
};