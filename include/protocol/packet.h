#pragma once

#include <Arduino.h>
#include <variant>
#include <utility>
#include <string>

enum class MessageType {
    Wake,
    Reset,
    PowerOff,
    Telemetry,
    Heartbeat,
    CommandResult,
    Unknown
};

enum class AckStatus {
    Ok,
    Error,
    InvalidCommand,
    Busy
};

struct Packet {
    std::string id;
    uint64_t timestamp;
};

struct Message : Packet {
    MessageType type;
    String body;
    std::string replyTo;

    Message(
        std::string id, 
        uint64_t timestamp, 
        MessageType type, 
        String body = "null",
        std::string replyTo = ""
    )
        : Packet{std::move(id), timestamp}, 
          type(type), 
          body(std::move(body)),
          replyTo(std::move(replyTo)) {}
};

struct Acknowledgement : Packet {
    AckStatus status;
    String error;

    Acknowledgement(
        std::string id,
        uint64_t timestamp, 
        AckStatus status, 
        String error = ""
    )
        : Packet{std::move(id), timestamp}, 
          status(status), 
          error(std::move(error)) {}
};

/**
 * Type-safe representation of all supported protocol packets.
 *
 * std::variant stores exactly one of the listed packet types.
 * Additional packet structures can be added as the protocol expands.
 */
using PacketVariant = std::variant<Message, Acknowledgement>;