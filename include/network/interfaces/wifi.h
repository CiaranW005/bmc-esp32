#pragma once 

#include "interface.h"

namespace bmc {

class WiFiInterface : public NetworkInterface {
public:
    WiFiInterface(Logger& logger, const char* ssid, const char* password)
        : NetworkInterface(logger), ssid_(ssid), password_(password) {}

    bool connect() override;
    bool isConnected() const override;
    void disconnect() override;

private:
    const char* ssid_;
    const char* password_;
};

}