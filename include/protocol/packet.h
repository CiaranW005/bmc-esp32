#pragma once

#include <Arduino.h>
#include <variant>
#include <utility>

enum class MessageType {
    Reset,
    PowerOff,
    Telemetry,
    Heartbeat,
    Unknown
};

enum class AckStatus {
    Ok,
    Error,
    InvalidCommand,
    Busy
};

struct Packet {
    uint64_t timestamp;
};

struct Message : Packet {
    MessageType type;
    String body;

    Message(uint64_t timestamp, MessageType type, String body)
        : Packet{timestamp}, type(type), body(std::move(body)) {}
};

struct Acknowledgement : Packet {
    AckStatus status;
    String error;

    Acknowledgement(uint64_t timestamp, AckStatus status, String error)
        : Packet{timestamp}, status(status), error(std::move(error)) {}
};

/**
 * Type-safe representation of all supported protocol packets.
 *
 * std::variant stores exactly one of the listed packet types.
 * Additional packet structures can be added as the protocol expands.
 */
using PacketVariant = std::variant<Message, Acknowledgement>;