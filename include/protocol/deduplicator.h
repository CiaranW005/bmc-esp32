#pragma once

#include <Arduino.h>
#include <unordered_map>
#include <string>
#include <cstdint>

class Deduplicator {
public:
    explicit Deduplicator(uint32_t lifetimeMs = 300000)
        : lifetimeMs_{lifetimeMs} {}
    
    // Record unseen IDs and return true for IDs already within the expiry window.
    // Repeated checks do not extend the original expiry time.
    bool isDuplicate(const std::string& id);

private:
    std::unordered_map<std::string, uint32_t> seen_;
    uint32_t lifetimeMs_;
};