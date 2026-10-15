#pragma once

#include "management_controller.h"
#include "protocol/protocol.h"

#include <Arduino.h>
#include <NetworkClient.h>
#include <NetworkUdp.h>

#include <mutex>
#include <optional>
#include <functional>

class NetworkManagementController: public ManagementController {
public:
    NetworkManagementController();
    void tick();
    
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
    
    void update();

    ManagementResult sendCommand(MessageType type, uint32_t timeoutMs, ResponseCallback complete);

    using CompletionHandler = 
        std::function<void(MessageType, const ResponseResult&)>;
    
    void setCompletionHandler(CompletionHandler handler);

private:
    NetworkUDP udp_;
    NetworkClient client_;
    
    std::mutex clientMutex_;

    AckTracker tracker_;
    ResponseTracker responses_;
    Deduplicator dedup_;
    String recieveBuffer_;

    CompletionHandler completion_;

    struct PendingWake { uint32_t deadline; };
    std::optional<PendingWake> pendingWake_;

    bool receive(String& message);

    void complete(MessageType type, const ResponseResult& result);

    template <typename T>
    bool send(const T& packet) {
        String payload = serialisePacket(packet);

        std::lock_guard<std::mutex> lock(clientMutex_);

        if (!client_.connected()) {
            logger_.error("Unable to write as ESP32 is currently not connected");
            return false;
        }

        size_t written = client_.print(payload);
        written += client_.print('\n');

        return written == payload.length() + 1;
    }
};