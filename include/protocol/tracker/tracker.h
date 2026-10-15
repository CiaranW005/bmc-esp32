#pragma once

#include <unordered_map>
#include <string>
#include <mutex>
#include <utility>

template <typename T>
class Tracker {
public:
    virtual ~Tracker() = default;

    bool track(const std::string& id, T entry) {
        if (id.empty()) {
            return false;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        return pending_.emplace(id, std::move(entry)).second;
    }

    virtual void cancel(const std::string& id) {
        std::lock_guard<std::mutex> lock(mutex_);
        pending_.erase(id);
    }

    virtual void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        pending_.clear();
    }

    bool isPending(const std::string& id) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return pending_.find(id) != pending_.end();
    }

    void tick() { 
        checkTimeouts(); 
    }

protected:
    virtual void checkTimeouts() = 0;
    
    mutable std::mutex mutex_;
    std::unordered_map<std::string, T> pending_;
};  