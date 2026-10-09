#pragma once

#include <stdint.h>
#include <stddef.h>
#include <WebSocketsClient.h>

#include "board/management_controller.h"

class OracleHandler {
public:
    OracleHandler(ManagementController& manager);

    void connect();
    void loop();

    bool isConnected();
    bool send(const String& message);

private:
    WebSocketsClient webSocket_;
    ManagementController& manager_;

    void handleEvent(WStype_t type, uint8_t* payload, size_t length);
};