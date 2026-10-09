#include "board/network_controller.h"
#include "config/secrets.h"
#include "protocol/protocol.h"

#include <ArduinoJson.h>

#include <WiFi.h>
#include <WiFiUdp.h>
#include <stdint.h>
#include <time.h>

ManagementPriority NetworkManagementController::priority() const {return ManagementPriority::Network;}

ManagementResult NetworkManagementController::powerOn() {
    uint8_t packet[102];

    // WoL magic packet: 6 bytes of 0xFF followed by 16 repetitions of the MAC.
    memset(packet, 0xFF, 6);

    for (int i = 1; i <= 16; i++) {
        memcpy(&packet[i * 6], PC_MAC, 6);
    }

    if (!udp_.beginPacket("255.255.255.255", 9)) {
        return ManagementResult::Failed;
    }

    if (udp_.write(packet, sizeof(packet)) != sizeof(packet)) {
        return ManagementResult::Failed;
    }

    if (!udp_.endPacket()) {
        return ManagementResult::Failed;
    }

    return ManagementResult::Success;
}

ManagementResult NetworkManagementController::powerOff() {
   return sendCommand(MessageType::PowerOff).result;
}

ManagementResult NetworkManagementController::reset() {
  return sendCommand(MessageType::Reset).result;
}

void NetworkManagementController::connect() {
    if (isConnected()) {
        logger_.error("ESP32 already connected to PC Agent");
        return;
    }

    std::lock_guard<std::mutex> lock(clientMutex_);
    if (client_.connect("192.168.1.67", 9090)) {
        logger_.info("Connected to PC Agent");
    } else {
        logger_.error("Failed to connect to PC Agent");
    }
}

void NetworkManagementController::disconnect() {
    if (!isConnected()) {
        logger_.error("ESP32 already disconnected from PC agent");
        return;
    }

    std::lock_guard<std::mutex> lock(clientMutex_);
    client_.stop();
}

bool NetworkManagementController::isConnected() {
    std::lock_guard<std::mutex> lock(clientMutex_);
    return client_.connected();
}

CommandResponse NetworkManagementController::sendCommand(MessageType type) {
    if (!isConnected()) {
        logger_.error("Unable to reboot PC as ESP32 is currently not connected");
        return {ManagementResult::Unavailable};
    }

    uint64_t timestamp = static_cast<uint64_t>(time(nullptr));
    Message msg = {
        timestamp,
        type,
        "ok"  
    };
    
    String raw;
    {
        std::lock_guard<std::mutex> lock(clientMutex_);

        if(!send(msg)) {
            logger_.error("Failed to send command to PC Agent");
            return {ManagementResult::Failed};
        }

        if(!receive(raw)) {
            logger_.error("Failed to receive response from PC Agent");
            return {ManagementResult::Failed};
        }
    }

    PacketVariant packet = deserialisePacket(raw);
    if (!std::holds_alternative<Acknowledgement>(packet)) {
        logger_.error("Expected acknowledgement from PC agent");
        return {ManagementResult::Failed};
    }

    const auto& ack = std::get<Acknowledgement>(packet); 
    if (ack.status != AckStatus::Ok) {
        logger_.error(ack.error.c_str());
        return {ManagementResult::Failed, ack};
    }
    
    return {ManagementResult::Success, packet};
}

bool NetworkManagementController::send(const Message& message) {
    if (!client_.connected()) {
        logger_.error("Unable to write as ESP32 is currently not connected");
        return false;
    }

    String payload = serialisePacket(message);

    size_t written = client_.print(payload);
    client_.print('\n');

    return written == payload.length();
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
