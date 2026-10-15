#pragma once

#include <esp_random.h>
#include <array>
#include <string>
#include <cstdio>

inline std::string generateID() {
    std::array<uint8_t, 16> bytes;

    esp_fill_random(bytes.data(), bytes.size());

    // UUID version 4
    bytes[6] = (bytes[6] & 0x0F) | 0x40;

    // RFC 4122 variant
    bytes[8] = (bytes[8] & 0x3F) | 0x80;

    char uuid[37];

    snprintf(
        uuid, sizeof(uuid),
        "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        bytes[0], bytes[1], bytes[2], bytes[3],
        bytes[4], bytes[5],
        bytes[6], bytes[7],
        bytes[8], bytes[9],
        bytes[10], bytes[11], bytes[12],
        bytes[13], bytes[14], bytes[15]
    );

    return std::string(uuid);
}