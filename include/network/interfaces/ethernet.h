#pragma once

#include "interface.h"

#include "logger/logger.h"

namespace bmc {

class EthernetInterface : public NetworkInterface {
public:
    EthernetInterface(Logger& logger): NetworkInterface(logger) {}

    bool connect() override;
    bool isConnected() const override;
    void disconnect() override;
};

}