#pragma once

#include <NetworkUdp.h>

#include "logger/transporter/transport.h"

namespace bmc {
    class NetworkManager;
}

class UdpLogTransport: public LogTransport {
public:
    bool begin(const char* host, uint16_t port = 5768);

    void setNetworkManager(const bmc::NetworkManager& manager);

    bool available() const override;
    bool send(const LogEntry& entry) override;

private:
    const bmc::NetworkManager* networkManager_ = nullptr;

    NetworkUDP udp_;
    IPAddress host_;
    uint16_t port_ = 5768;
};