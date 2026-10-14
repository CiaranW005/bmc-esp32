#include "logger/transporter/transport.h"
#include "network/manager.h"

#include <Arduino.h>

bool UdpLogTransport::begin(const char* host, uint16_t port) {
    if (!host_.fromString(host)) {
        Serial.printf("[UDP][ERROR] Invalid host: %s\n", host);
        return false;
    }

    port_ = port;

    Serial.printf(
        "[UDP][INFO] Destination configured: %s:%u\n",
        host,
        port
    );

    return true;
}

bool UdpLogTransport::available() const {
    return networkManager_ && networkManager_->isConnected();
}

bool UdpLogTransport::send(const LogEntry& entry) {
    if (!available()) return false;

    char buffer[256];

    int length = snprintf(
        buffer,
        sizeof(buffer),
        "[%lu][%s][%s] %s\n",
        static_cast<unsigned long>(entry.timestamp),
        entry.component,
        entry.level == LogLevel::ERROR ? "ERROR" : "INFO",
        entry.message
    );

    if (length < 0) {
        Serial.println("[UDP][ERROR] Message formatting failed");
        return false;
    }

    if (!udp_.beginPacket(host_, port_)) {
        Serial.println("[UDP][ERROR] beginPacket() failed");
        return false;
    }

    size_t bytes = strlen(buffer);

        
    size_t written = udp_.write(
        reinterpret_cast<const uint8_t*>(buffer),
        bytes
    );

    if (written != bytes) {
        Serial.printf(
            "[UDP][ERROR] Incomplete write: %u/%u bytes\n",
            static_cast<unsigned>(written),
            static_cast<unsigned>(bytes)
        );
        return false;
    }

    if (udp_.endPacket() != 1) {
        Serial.println("[UDP][ERROR] endPacket() failed");
        return false;
    }

    return true;
}

void UdpLogTransport::setNetworkManager(const bmc::NetworkManager& manager) {
    networkManager_ = &manager;
}