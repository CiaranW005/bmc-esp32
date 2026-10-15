#pragma once

#include "tracker.h"
#include "protocol/packet.h"

#include <cstdint>
#include <functional>
#include <optional>

enum class ResponseError {
    None,
    Timeout,
    Canceled,
    DeliveryFailed
};

struct ResponseResult {
    std::optional<Message> message;
    ResponseError err = ResponseError::None;
};

using ResponseCallback =
    std::function<void(const ResponseResult&)>;

struct PendingResponse {
    Message message;
    uint32_t deadline;
    ResponseCallback result;
};

class ResponseTracker : public Tracker<PendingResponse> {
public:
    bool track(
        const Message& message,
        uint32_t timeoutMs,
        ResponseCallback result
    );

    bool resolve(const Message& message);

    void fail(const std::string& id, ResponseError error);
    void cancel(const std::string& id) override;
    void clear() override;

protected:
    void checkTimeouts() override;
};