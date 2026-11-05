#include "oracle/oracle_handler.h"
#include "config/secrets.h"
#include "config/config.h"
#include "config/device_credentials.h"
#include "controllers/management_controller.h"

#include <ArduinoJson.h>
#include <ctime>

#include  <WiFi.h>
#include <WebSocketsClient.h>

OracleHandler::OracleHandler(ManagementController& manager)
    : manager_(manager),
      tracker_(TrackerCallbacks{
          [this](const Message& message) -> bool {
              return send(serialisePacket(message));
          },
          [this](const Message& message) {
              logger_.error("Message acknowledgement timed out");
          },
          [this](const Message& message, const Acknowledgement& ack) {
              logger_.info("Message acknowledged");
          }
      }) {}

void OracleHandler::begin() {
#ifdef ORACLE_LOCAL
    webSocket_.begin(
        ORACLE_HOST,
        ORACLE_PORT,
        "/device"
    );
#else
    webSocket_.beginSslWithClientKey(
        ORACLE_HOST,
        ORACLE_PORT,
        "/device",
        BMC_ROOT_CA,
        CLIENT_CERT,
        CLIENT_KEY
    );
#endif

    String headers;

#ifdef ORACLE_LOCAL
    headers = "Authorization: Bearer " + String(DEVICE_TOKEN);
#else
    headers =
        "Origin: https://device.corasfein.com:443\r\n"
        "Authorization: Bearer " + String(DEVICE_TOKEN);
#endif

    webSocket_.setExtraHeaders(headers.c_str());

    webSocket_.onEvent(
        [this](WStype_t type, uint8_t* payload, size_t length) {
            this->handleEvent(type, payload, length);
        }
    );
    
    webSocket_.setReconnectInterval(5000);
}

void OracleHandler::disconnect() {
    webSocket_.disconnect();
    tracker_.clear();
}

void OracleHandler::loop() {
    webSocket_.loop();
    tracker_.tick();

    if (!isConnected()) return;

    std::optional<Message> reply;

    {
        std::lock_guard<std::mutex> lock (commandMutex_);

        if (!replies_.empty()) {
            reply = std::move(replies_.front());
            replies_.pop_front();
        }
    }

    if (!reply) return;

    if (!tracker_.track(*reply, RetryPolicy{1000, 2})) {
        logger_.error("Failed to track Oracle response");
        return;
    }

    if (!send(serialisePacket(*reply))) {
        logger_.error("Oracle response send failed; retry scheduled");
    }
}

bool OracleHandler::isConnected() {
    return webSocket_.isConnected();
}

bool OracleHandler::send(const String& message) {
    if (!this->isConnected()) {
        return false;
    }

    return webSocket_.sendTXT(message.c_str());
}

void OracleHandler::processCommand() {
    std::optional<Message> request;

    // Claim the queued command under the mutex, then execute outside the lock.
    // Success means the operation was accepted; completion may arrive later.
    {
        std::lock_guard<std::mutex> lock(commandMutex_);

        if (!pendingCommand_ || commandStarted_) return;

        commandStarted_ = true;
        request = pendingCommand_;
    }

    ManagementResult result = ManagementResult::Failed;

    switch (request->type) {
        case MessageType::Wake:
            result = manager_.powerOn();
            break;
        
        case MessageType::Reset:
            result = manager_.reset();
            break;
        
        case MessageType::PowerOff:
            result = manager_.powerOff();
            break;
        
        default:
            break;
    }

    if (result != ManagementResult::Success) {
        commandCompleted(
            request->type,
            {std::nullopt, ResponseError::DeliveryFailed}
        );
    }
}

void OracleHandler::commandCompleted(
    MessageType type,
    const ResponseResult& result
) {
    std::optional<Message> request;

    {
        std::lock_guard<std::mutex> lock(commandMutex_);

        if (!pendingCommand_ ||
            !commandStarted_ ||
            pendingCommand_->type != type) {
            return;
        }

        request = std::move(pendingCommand_);
        pendingCommand_.reset();
        commandStarted_ = false;
    }

    // Forward the PC agent's response body.
    if (result.err == ResponseError::None && result.message) {
        queueReply(*request, result.message->body);
        return;
    }

    // Suppress response timeouts for reset and power-off because the PC agent
    // may disconnect as the requested operation takes effect.
    if (result.err == ResponseError::Timeout &&
        type != MessageType::Wake) {
        return;
    }

    JsonDocument body;
    body["success"] = result.err == ResponseError::None;

    switch (result.err) {
        case ResponseError::None:
            break;

        case ResponseError::Timeout:
            body["error"] = "PC agent did not come online";
            break;

        case ResponseError::DeliveryFailed:
            body["error"] = "Could not deliver command to PC agent";
            break;

        case ResponseError::Canceled:
            body["error"] = "Command canceled";
            break;
    }

    String json;
    serializeJson(body, json);
    queueReply(*request, json);
}

std::optional<Message> OracleHandler::takeTelemetryRequest() {
    std::lock_guard<std::mutex> lock(commandMutex_);

    auto request = std::move(telemetryRequest_);
    telemetryRequest_.reset();
    return request;
}

void OracleHandler::telemetryCompleted(
    const Message& request,
    const ResponseResult& result
) {
    // Report the PC as unavailable unless the agent returns valid telemetry
    // containing a boolean "pc" field.
    bool pcAvailable = false;

    if (result.err == ResponseError::None &&
        result.message &&
        result.message->type == MessageType::Telemetry) {
        JsonDocument body;

        if (!deserializeJson(body, result.message->body) &&
            body["pc"].is<bool>()) {
            pcAvailable = body["pc"].as<bool>();
        }
    }

    JsonDocument body;
    body["pc"] = pcAvailable;

    String json;
    serializeJson(body, json);

    // queueReply uses request.id as the outgoing replyTo.
    queueReply(request, json);
}

bool OracleHandler::queueCommand(const Message& request) {
    std::lock_guard<std::mutex> lock(commandMutex_);

    if (pendingCommand_) return false;

    pendingCommand_ = request;
    commandStarted_ = false;
    return true;
}

void OracleHandler::queueReply(
    const Message& request,
    const String& body
) {
    Message response{
        generateID(),
        static_cast<uint64_t>(time(nullptr)),
        request.type,
        body,
        request.id
    };
    
    // Keep only the latest telemetry request waiting for the controller task.
    // A new request replaces any request that has not yet been taken.
    std::lock_guard<std::mutex> lock(commandMutex_);
    replies_.push_back(std::move(response));
}

void OracleHandler::queueTelemetry(const Message& request) {
    std::lock_guard<std::mutex> lock(commandMutex_);
    telemetryRequest_ = request;
}

void OracleHandler::handleEvent(WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_CONNECTED:
            logger_.info("Connected to Oracle");
            break;
        
        case WStype_DISCONNECTED:
            logger_.info("Disconnected from Oracle");
            tracker_.clear();
            break;
        
        case WStype_TEXT: {
            String raw((char*)payload, length);

            PacketVariant packet = deserialisePacket(raw);

            std::visit([this](const auto& p) {
                using T = std::decay_t<decltype(p)>;

                if constexpr (std::is_same_v<T, Acknowledgement>) {
                    if (p.id.empty()) {
                        logger_.error("Invalid Oracle packet");
                        return;
                    }

                    tracker_.acknowledge(p);
                }
                else if constexpr (std::is_same_v<T, Message>) {
                    if (p.id.empty()) {
                        logger_.error("Oracle message missing ID");
                        return;
                    }

                    logger_.info(("Oracle command: " + String(toString(p.type))).c_str());

                    bool valid = true;

                    switch (p.type) {
                        case MessageType::Wake:
                        case MessageType::Reset:
                        case MessageType::PowerOff:
                        case MessageType::Heartbeat:
                        case MessageType::Telemetry:
                            break;

                        default:
                            valid = false;
                            break;
                    }

                    if (!valid) {
                        Acknowledgement ack{
                            p.id,
                            millis(),
                            AckStatus::InvalidCommand
                        };

                        send(serialisePacket(ack));
                        return;
                    }
                    
                    // Acknowledge retransmissions again so the sender can stop retrying,
                    // but process each message ID only once within the deduplication window.
                    bool duplicate = dedup_.isDuplicate(p.id);

                    Acknowledgement ack{
                        p.id,
                        millis(),
                        AckStatus::Ok
                    };

                    if (!send(serialisePacket(ack))) {
                        logger_.error("Failed to send acknowledgement");
                        return;
                    }

                    if (duplicate) return;

                    if (p.type == MessageType::Telemetry) {
                        queueTelemetry(p);
                        return;
                    }

                    if (!queueCommand(p)) {
                        queueReply(
                            p,
                            "{\"success\":false,\"error\":\"Another power command is pending\"}"
                        );
                    }
                }
            }, packet);
                break;
        }

        case WStype_ERROR:
            logger_.error("Websocket error");
            break;

        default:
            break;
    }
}
