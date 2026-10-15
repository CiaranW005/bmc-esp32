
#include "protocol/tracker/response.h"

#include <vector>

bool ResponseTracker::track(
    const Message& message,
    uint32_t timeoutMs,
    ResponseCallback result
) {
    if (message.id.empty() || timeoutMs == 0) {
        return false;
    }

    PendingResponse entry{
        message,
        millis() + timeoutMs,
        std::move(result)   
    };

    return Tracker<PendingResponse>::track(
        message.id,
        std::move(entry)
    );
}

bool ResponseTracker::resolve(const Message& message) {
    ResponseCallback callback;

    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = pending_.find(message.replyTo);
        if (it == pending_.end()) return false;

        callback = std::move(it->second.result);
        pending_.erase(it);
    }

    if (callback) {
        callback(
            ResponseResult{
                message,
                ResponseError::None
            });
    }

    return true;
}

void ResponseTracker::fail(const std::string& id, ResponseError error) {
    ResponseCallback callback;

    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = pending_.find(id);
        if (it == pending_.end()) return;

        callback = std::move(it->second.result);
        pending_.erase(it);
    }

    if (callback) {
        callback(ResponseResult{
            std::nullopt,
            error
        });
    }
}

void ResponseTracker::cancel(const std::string& id) {
    fail(id, ResponseError::Canceled);
}

void ResponseTracker::clear() {
    std::vector<ResponseCallback> callbacks;

    {
        std::lock_guard<std::mutex> lock(mutex_);

        for (auto it = pending_.begin(); it != pending_.end();) {
            callbacks.emplace_back(std::move(it->second.result));
            it = pending_.erase(it);
        }
    }

    for (const ResponseCallback& cb : callbacks) {
        if (cb) {
            cb(ResponseResult{
                std::nullopt,
                ResponseError::Canceled
            });
        }
    }
}

void ResponseTracker::checkTimeouts() {
    const uint32_t now = millis();
    std::vector<ResponseCallback> callbacks;

    {
        std::lock_guard<std::mutex> lock(mutex_);

        for (auto it = pending_.begin(); it != pending_.end();) {
            PendingResponse& pending = it->second;

            if (static_cast<int32_t>(now - pending.deadline) >= 0) {
                callbacks.emplace_back(std::move(pending.result));
                it = pending_.erase(it);
                continue;
            }
            ++it;
        }
    }

    for (const ResponseCallback& cb : callbacks) {
        if (cb) {
            cb(ResponseResult{
                std::nullopt,
                ResponseError::Timeout
            });
        }
    }
}
