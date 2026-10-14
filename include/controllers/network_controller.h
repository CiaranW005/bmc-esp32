#pragma once

#include "management_controller.h"
#include "protocol/packet.h"

#include <Arduino.h>
#include <NetworkClient.h>
#include <NetworkUdp.h>

#include <mutex>
#include <optional>

struct CommandResponse {
    ManagementResult result;
    std::optional<PacketVariant> packet = std::nullopt;
};

class NetworkManagementController: public ManagementController {
public:
    NetworkManagementController(): ManagementController("NET_CTRL") {}

    ManagementPriority priority() const override;
    
    /**
     * Sends a Wake-on-LAN magic packet to the configured PC.
     * Success indicates transmission, not confirmed startup.
     */
    ManagementResult powerOn() override;
    
    /**
     * Requests a graceful shutdown through the PC agent.
     * Requires an active connection and acknowledgement.
     */
    ManagementResult powerOff() override;

    /**
     * Requests a graceful reboot through the PC agent.
     * Requires an active connection and acknowledgement.
     */
    ManagementResult reset() override;

    void connect();
    void disconnect();
    bool isConnected();
    
    CommandResponse sendCommand(MessageType type);

private:
    NetworkUDP udp_;
    NetworkClient client_;
    
    std::mutex clientMutex_;

    bool send(const Message& message);
    bool receive(String& message);
};