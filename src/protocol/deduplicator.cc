#include "protocol/deduplicator.h"

bool Deduplicator::isDuplicate(const std::string& id) {
    uint32_t now = millis();

    for (auto it = seen_.begin(); it != seen_.end();) {
        if (static_cast<int32_t>(now - it->second) >= 0) {
            it = seen_.erase(it);
        } else {
            ++it;
        }
    }

    auto [it, inserted] = seen_.emplace(id, now + lifetimeMs_);
    return !inserted;
}