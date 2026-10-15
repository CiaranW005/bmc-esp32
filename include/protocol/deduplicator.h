#pragma once

#include <Arduino.h>
#include <unordered_map>
#include <string>
#include <cstdint>

class Deduplicator {
public:
    explicit Deduplicator(uint32_t lifetimeMs = 300000)
        : lifetimeMs_{lifetimeMs} {}

    bool isDuplicate(const std::string& id);

private:
    std::unordered_map<std::string, uint32_t> seen_;
    uint32_t lifetimeMs_;
};