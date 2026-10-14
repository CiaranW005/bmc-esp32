
#include "network/manager.h"

#include <Arduino.h>

namespace bmc {

void NetworkManager::begin() {

    if (ethernet_.connect()) {
        unsigned long start = millis();

        while (!ethernet_.isConnected() &&
            millis() - start < 3000) {
            delay(100);
        }

        if (ethernet_.isConnected()) {
            setActive(&ethernet_);
            return;
        }
    }

    if (wifi_.connect()) {
        setActive(&wifi_);
        return;
    }

    setActive(nullptr);
}

void NetworkManager::update() {
    if (ethernet_.isConnected()) {
        if(active_ != &ethernet_) {
            setActive(&ethernet_);
            logger_.info("Switched to Ethernet");
        }
    }
    else if (wifi_.isConnected()) {
        if (active_ != &wifi_) {
            setActive(&wifi_);
            logger_.info("Switched to WiFi");
        }
    }
    else {
        if (active_ != nullptr) {
            setActive(nullptr);
            logger_.error("All connections lost");
        }
    }
}

bmc::NetworkInterface* NetworkManager::getActiveInterface() const {
    return active_;
}

bool NetworkManager::isConnected() const {
    auto* interface = active_.load();

    return interface != nullptr && interface->isConnected();
}

uint32_t NetworkManager::generation() const {
    return generation_.load();
}

void NetworkManager::setActive(bmc::NetworkInterface* interface) {
    if (active_.load() == interface) return;

    active_.store(interface);
    generation_.fetch_add(1);
}
}
