#pragma once

#include "packet.h"
#include "enum_traits.h"

#include <ArduinoJson.h>
#include <type_traits>

/**
 * Serialises a protocol packet into JSON.
 *
 * if constexpr selects the serialisation logic at compile time
 * based on the packet's concrete type.
 *
 * Additional packet types can be supported by extending the
 * compile-time branches below.
 */
template <typename T>
inline String serialisePacket(const T& packet) {
    JsonDocument doc;

    doc["timestamp"] = packet.timestamp;

    if constexpr (std::is_same_v<T, Message>) {
        doc["type"] = toString(packet.type);
        doc["body"] = packet.body;
    }
    else if constexpr (std::is_same_v<T, Acknowledgement>) {
        doc["status"] = toString(packet.status);
        doc["error"] = packet.error;
    }

    String output;
    serializeJson(doc, output);

    return output;
}

/**
 * Deserialises JSON into one of the supported packet types.
 *
 * Packet identification:
 * - "status" field -> Acknowledgement
 * - "type" field   -> Message
 *
 * Invalid JSON or unrecognized packet structures produce
 * an error Acknowledgement.
 */
inline PacketVariant deserialisePacket(const String& raw) {
    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, raw);

    if (error) {
        return Acknowledgement{
            0,
            AckStatus::Error,
            "Invalid JSON"
        };
    }

    uint64_t timestamp = doc["timestamp"] | uint64_t{0};

    if (doc["status"].is<const char*>()) {
        return Acknowledgement{
            timestamp,
            fromString(
                doc["status"].as<String>(),
                AckStatus::InvalidCommand
            ),
            doc["error"].as<String>()
        };
    }

    if (doc["type"].is<const char*>()) {
        return Message{
            timestamp,
            fromString(
                doc["type"].as<String>(),
                MessageType::Unknown
            ),
            doc["body"].as<String>()
        };
    }

    return Acknowledgement{
        timestamp,
        AckStatus::InvalidCommand,
        "Packet contained neither type nor status"
    };
}