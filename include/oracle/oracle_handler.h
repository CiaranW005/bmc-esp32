#pragma once

#include <stdint.h>
#include <stddef.h>
#include <deque>
#include <mutex>
#include <optional>
#include <WebSocketsClient.h>

#include "controllers/management_controller.h"
#include "protocol/protocol.h"
#include "logger/logger.h"

class OracleHandler {
public:
    OracleHandler(ManagementController& manager);

    void begin();
    void disconnect();
    void loop();

    bool isConnected();
    bool send(const String& message);

    void processCommand();
    void commandCompleted(MessageType type, const ResponseResult& result);

    std::optional<Message> takeTelemetryRequest();
    void telemetryCompleted(const Message& request, const ResponseResult& result);

private:
    WebSocketsClient webSocket_;
    ManagementController& manager_;

    AckTracker tracker_;
    Deduplicator dedup_;

    Logger logger_{"ORACLE"};

    std::mutex commandMutex_;

    std::optional<Message> pendingCommand_;
    bool commandStarted_ = false;

    std::optional<Message> telemetryRequest_;

    std::deque<Message> replies_;

    bool queueCommand(const Message& request);
    void queueReply(const Message& request, const String& body);
    void queueTelemetry(const Message& request);

    void handleEvent(WStype_t type, uint8_t* payload, size_t length);
};