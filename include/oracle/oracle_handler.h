#pragma once

#include <stdint.h>
#include <stddef.h>
#include <WebSocketsClient.h>

#include "controllers/management_controller.h"
#include "logger/logger.h"

class OracleHandler {
public:
    OracleHandler(ManagementController& manager);

    void begin();
    void disconnect();
    void loop();

    bool isConnected();
    bool send(const String& message);

private:
    WebSocketsClient webSocket_;
    ManagementController& manager_;

    Logger logger_{"ORACLE"};

    void handleEvent(WStype_t type, uint8_t* payload, size_t length);
};