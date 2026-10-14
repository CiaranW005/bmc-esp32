#include "interfaces/interface.h"
#include "interfaces/ethernet.h"
#include "interfaces/wifi.h"

#include "logger/logger.h"

#include <atomic>

namespace bmc {

class NetworkManager {
public:
    NetworkManager(const char* ssid, const char* password)
        : ethernet_(logger_), wifi_(logger_, ssid, password) {}

    void begin();
    void update();

    bmc::NetworkInterface* getActiveInterface() const;
    bool isConnected() const;

    uint32_t generation() const;

private:
    Logger logger_{"NETWORK"};
    
    bmc::EthernetInterface ethernet_;
    bmc::WiFiInterface wifi_;

    std::atomic<bmc::NetworkInterface*> active_{nullptr};
    std::atomic<uint32_t> generation_{0};

    void setActive(bmc::NetworkInterface* interface);
};

}