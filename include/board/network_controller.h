#pragma once

#include "management_controller.h"
#include "protocol/packet.h"

#include <mutex>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

struct CommandResponse {
    ManagementResult result;
    std::optional<PacketVariant> packet = std::nullopt;
};

class NetworkManagementController: public ManagementController {
public:
    NetworkManagementController(): ManagementController("NETWORK") {}

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
    WiFiUDP udp_;
    WiFiClient client_;

    std::mutex clientMutex_;

    bool send(const Message& message);
    bool receive(String& message);
};