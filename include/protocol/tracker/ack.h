#pragma once

#include "tracker.h"
#include "protocol/packet.h"

#include <Arduino.h>
#include <functional>
#include <string>
#include <unordered_map>

struct RetryPolicy {
    uint32_t timeoutMs = 3000;
    uint8_t maxRetries = 0;
};

struct PendingMessage {
    Message message;
    RetryPolicy policy;
    uint32_t deadline;
    uint8_t retries = 0;
};

struct TrackerCallbacks {
    std::function<bool(const Message&)> retry;
    std::function<void(const Message&)> failed;
    std::function<void(const Message&, const Acknowledgement&)> acked;
};

class AckTracker : public Tracker<PendingMessage> {
public:
    explicit AckTracker(TrackerCallbacks callbacks)
        : callbacks_(std::move(callbacks)) {}

    bool track(const Message& message, RetryPolicy policy);
    bool acknowledge(const Acknowledgement& ack);

protected:
    void checkTimeouts() override;

private:
    TrackerCallbacks callbacks_;
};