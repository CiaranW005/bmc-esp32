#include "controllers/network_controller.h"
#include "config/secrets.h"
#include "protocol/protocol.h"

#include <ArduinoJson.h>

#include <WiFi.h>
#include <WiFiUdp.h>
#include <stdint.h>
#include <time.h>

NetworkManagementController::NetworkManagementController()
    : ManagementController("NET_CTRL"),
      tracker_(TrackerCallbacks{
            [this](const Message& message) -> bool {
                return send(message);
            },
           
            [this](const Message& message) {
                responses_.fail(message.id, ResponseError::DeliveryFailed);
            },

            [this](const Message& message, const Acknowledgement& ack) {
                if (ack.status != AckStatus::Ok) {
                    responses_.fail(message.id, ResponseError::DeliveryFailed);
                }
            }
      }) {}

ManagementPriority NetworkManagementController::priority() const {return ManagementPriority::Network;}

ManagementResult NetworkManagementController::powerOn() {
    if (pendingWake_) {
        return ManagementResult::Unavailable;
    }

    if (isConnected()) {
        complete(
            MessageType::Wake,
            {std::nullopt, ResponseError::None}
        );
        return ManagementResult::Success;
    }

    pendingWake_ = PendingWake{millis() + 50000};

    uint8_t packet[102];

    // WoL magic packet: 6 bytes of 0xFF followed by 16 repetitions of the MAC.
    memset(packet, 0xFF, 6);

    for (int i = 1; i <= 16; i++) {
        memcpy(&packet[i * 6], PC_MAC, 6);
    }

    if (!udp_.beginPacket("255.255.255.255", 9)) {
        pendingWake_.reset();
        return ManagementResult::Failed;
    }

    if (udp_.write(packet, sizeof(packet)) != sizeof(packet)) {
        pendingWake_.reset();
        return ManagementResult::Failed;
    }

    if (!udp_.endPacket()) {
        pendingWake_.reset();
        return ManagementResult::Failed;
    }

    return ManagementResult::Success;
}

ManagementResult NetworkManagementController::powerOff() {
   return sendCommand(
    MessageType::PowerOff, 
    4000, 
    [this](const ResponseResult& result) {
        complete(MessageType::PowerOff, result);
    });
}

ManagementResult NetworkManagementController::reset() {
  return sendCommand(
    MessageType::Reset,
    4000,
    [this](const ResponseResult& result) {
        complete(MessageType::Reset, result);
    });
}

void NetworkManagementController::connect() {
    if (isConnected()) return;

    bool connected;
    {
        std::lock_guard<std::mutex> lock(clientMutex_);
        connected = client_.connect("192.168.1.67", 9090, 250);
    } 

    if (!connected) {
        logger_.error("Failed to connect to PC Agent");
        return;
    }

    logger_.info("Connected to PC Agent");

    if (pendingWake_) {
        const bool expired =
            static_cast<int32_t>(
                millis() - pendingWake_->deadline
            ) >= 0;

        pendingWake_.reset();

        complete(
            MessageType::Wake,
            {
                std::nullopt,
                expired ? ResponseError::Timeout
                        : ResponseError::None
            }
        );
    }
}

void NetworkManagementController::disconnect() {
    if (!isConnected()) {
        logger_.error("ESP32 already disconnected from PC agent");
        return;
    }

    std::lock_guard<std::mutex> lock(clientMutex_);
    client_.stop();
    recieveBuffer_ = "";
    
    logger_.info("Disconnected from PC Agent");
}

bool NetworkManagementController::isConnected() {
    std::lock_guard<std::mutex> lock(clientMutex_);
    return client_.connected();
}

void NetworkManagementController::tick() {
    tracker_.tick();
    responses_.tick();

    if (pendingWake_ && 
        static_cast<int32_t>(
            millis() - pendingWake_->deadline
        ) >= 0) {
            pendingWake_.reset();

            complete(
                MessageType::Wake,
                {std::nullopt, ResponseError::Timeout}
            );
        }
}

void NetworkManagementController::update() {
    String raw;
    {
        std::lock_guard<std::mutex> lock(clientMutex_);
        if (!client_.connected()) return;

        while (client_.available()) {
            char c = static_cast<char>(client_.read());

            if (c == '\n') {
                raw = recieveBuffer_;
                recieveBuffer_ = "";
                break;
            }

            recieveBuffer_ += c;
        }
    }

    if (raw.isEmpty()) return;

    PacketVariant packet = deserialisePacket(raw);

    std::visit([this](const auto& p) {
        using T = std::decay_t<decltype(p)>;

        if constexpr (std::is_same_v<T, Acknowledgement>) {
            if (!p.id.empty()) tracker_.acknowledge(p);
        }
        else if constexpr (std::is_same_v<T, Message>) {
            if (p.id.empty()) {
                return;
            }

            if (p.replyTo.empty() ||
                (p.type != MessageType::Reset &&
                 p.type != MessageType::PowerOff &&
                 p.type != MessageType::Telemetry)) {
                send(Acknowledgement{
                    p.id,
                    static_cast<uint64_t>(time(nullptr)),
                    AckStatus::InvalidCommand
                });
                return;
            }

            bool duplicate = dedup_.isDuplicate(p.id);

            if (!send(Acknowledgement{
                p.id,
                static_cast<uint64_t>(time(nullptr)),
                AckStatus::Ok
            })) {
                logger_.error("Failed to acknowledge PC agent");
                return;
            }

            if (duplicate) return;

            responses_.resolve(p);
        }
    }, packet); 
}

ManagementResult NetworkManagementController::sendCommand(
    MessageType type,
    uint32_t timeoutMs,
    ResponseCallback complete
) {
    if (!isConnected()) return ManagementResult::Unavailable;
    
    Message message{
        generateID(),
        static_cast<uint64_t>(time(nullptr)),
        type,
        "null"
    };

    if (!responses_.track(message, timeoutMs, std::move(complete))) return ManagementResult::Failed;

    if (!tracker_.track(message, RetryPolicy{3000, 0})) {
        responses_.cancel(message.id);
        return ManagementResult::Failed;
    }

    if (!send(message)) {
        tracker_.cancel(message.id);
        responses_.cancel(message.id);
        return ManagementResult::Failed;
    }

    return ManagementResult::Success;
}

void NetworkManagementController::setCompletionHandler(CompletionHandler handler) {
    completion_ = std::move(handler);
}

bool NetworkManagementController::receive(String& payload) {
    if (!client_.connected()) {
        logger_.error("Unable to read as ESP32 is currently not connected");
        return false;
    }

    if (!client_.available()) {
        logger_.error("Unable to read as Client is currently not available");
        return false;
    }

    payload = client_.readStringUntil('\n');
    return true;
}

void NetworkManagementController::complete(MessageType type, const ResponseResult& result) {
    if (completion_) completion_(type, result);
}