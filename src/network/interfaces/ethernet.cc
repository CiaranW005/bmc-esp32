#include "network/interfaces/ethernet.h"

#include <Arduino.h>
#include <SPI.h>
#include <ETH.h>

// GPIO mapping for the external W5500 SPI Ethernet module.
// ETH_RST = -1 means no reset GPIO is supplied to the driver.
#define ETH_INT   9
#define ETH_CS    10
#define ETH_SCK   12
#define ETH_MISO  13
#define ETH_MOSI  11
#define ETH_RST   -1

namespace bmc {
    
bool EthernetInterface::connect() {
    SPI.begin(ETH_SCK, ETH_MISO, ETH_MOSI, ETH_CS);

    logger_.info("Initialising W5500...");

    if (!ETH.begin(
        ETH_PHY_W5500,
        1,          // PHY address
        ETH_CS,
        ETH_INT,
        ETH_RST,
        SPI
    )) {
        logger_.error("Failed to initialise W5500");
        return false;
    }

    logger_.info("W5500 initialised");
    return true;
}

bool EthernetInterface::isConnected() const {
    return ETH.linkUp() && ETH.hasIP();
}

void EthernetInterface::disconnect() {
    ETH.end();
}

}