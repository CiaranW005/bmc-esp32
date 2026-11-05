#include "network/interfaces/wifi.h"

#include <WiFi.h>

namespace bmc {

bool WiFiInterface::connect() {
    WiFi.begin(ssid_, password_);

    constexpr unsigned long TIMEOUT_MS = 10000;
    unsigned long start = millis();

    logger_.info("Connecting to WiFi");

    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start > TIMEOUT_MS) {
            logger_.error("WiFi connection timed out");
            WiFi.disconnect();
            return false;
        }
        
        delay(500);
    }

    logger_.info("Connected to WiFi");

    char message[64];

    snprintf(
        message,
        sizeof(message),
        "WiFi IP: %s",
        WiFi.localIP().toString().c_str()
    );

    logger_.info(message);

    return true;
}

bool WiFiInterface::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

void WiFiInterface::disconnect() {
    WiFi.disconnect();
}

}