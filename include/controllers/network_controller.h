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
     * Starts a Wake-on-LAN attempt unless the PC agent is already connected.
     * Success means the attempt was accepted, not that startup is confirmed.
     * The completion handler reports connection success or timeout.
     */
    ManagementResult powerOn() override;
    
    /**
     * Requests a graceful shutdown through the PC agent.
     * Requires an active PC agent connection.
     * Success means the request was sent; delivery and results are handled later.
     */
    ManagementResult powerOff() override;

    /**
     * Requests a graceful reboot through the PC agent.
     * Requires an active PC agent connection.
     * Success means the request was sent; delivery and results are handled later.
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