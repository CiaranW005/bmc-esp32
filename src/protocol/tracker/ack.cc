#include "protocol/tracker/ack.h"

#include <optional>
#include <vector>

bool AckTracker::track(const Message& message, RetryPolicy policy) {
    if (message.id.empty() || policy.timeoutMs == 0) {
        return false;
    }

    PendingMessage entry{
        message,
        policy,
        millis() + policy.timeoutMs,
        0
    };

    return Tracker<PendingMessage>::track(
        message.id,
        std::move(entry)
    );
}

bool AckTracker::acknowledge(const Acknowledgement& ack) {
    std::optional<Message> message;

    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = pending_.find(ack.id);
        if (it == pending_.end()) return false;

        message = std::move(it->second.message);
        pending_.erase(it);
    }
    

    if (callbacks_.acked) {
        callbacks_.acked(*message, ack);
    }

    return true;
}

void AckTracker::checkTimeouts() {
    const uint32_t now = millis();
    
    std::vector<Message> retries;
    std::vector<Message> failures;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        
        for (auto it = pending_.begin(); it != pending_.end();) {
            PendingMessage& pending = it->second;

            if (static_cast<int32_t>(now - pending.deadline) < 0) {
                it++;
                continue;
            }

            if (pending.retries >= pending.policy.maxRetries) {
                Message message = pending.message;
                failures.emplace_back(message);
                it = pending_.erase(it);

                continue;
            }

            pending.retries++;
            pending.deadline = now + pending.policy.timeoutMs;

            retries.emplace_back(pending.message);
            ++it;
        }
    }

    for (const Message& message : retries) {
        if (!isPending(message.id)) continue;

        if (!callbacks_.retry || !callbacks_.retry(message)) {
            bool removed;
            {
                std::lock_guard<std::mutex> lock(mutex_);
                removed = pending_.erase(message.id) != 0;
            }

            if (removed && callbacks_.failed) callbacks_.failed(message);
        }
    }

    for (const Message& message : failures) {
        if (callbacks_.failed) callbacks_.failed(message);
    }
}


