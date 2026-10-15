#pragma once

#include "packet.h"

/**
 * Associates an enum value with its protocol string representation.
 */
template <typename EnumType>
struct EnumStringEntry {
    EnumType value;
    const char* name;
};

constexpr EnumStringEntry<MessageType> MESSAGE_TYPES[] = {
    {MessageType::Wake,          "wake"},
    {MessageType::Reset,         "reset"},
    {MessageType::PowerOff,      "power_off"},
    {MessageType::Telemetry,     "telemetry"},
    {MessageType::Heartbeat,     "heartbeat"},
    {MessageType::CommandResult, "command_result"},
    {MessageType::Unknown,       "unknown"},
};

constexpr EnumStringEntry<AckStatus> ACK_STATUSES[] = {
    {AckStatus::Ok,             "ok"},
    {AckStatus::Error,          "error"},
    {AckStatus::InvalidCommand, "invalid_command"},
    {AckStatus::Busy,           "busy"},
};

/**
 * Compile-time mapping between an enum type and its string lookup table.
 *
 * Each supported enum specializes EnumTraits to expose its entries().
 * This allows toString() and fromString() to work with different enums
 * without duplicating their implementations.
 *
 * The primary template is intentionally undefined, so unsupported
 * enum types cannot use these conversion functions.
 */
template <typename EnumType>
struct EnumTraits;

template <>
struct EnumTraits<MessageType> {
    // decltype((...)) preserves the array reference rather than copying it.
    static constexpr auto entries() -> decltype((MESSAGE_TYPES)) {
        return MESSAGE_TYPES;
    }
};

template <>
struct EnumTraits<AckStatus> {
    static constexpr auto entries() -> decltype((ACK_STATUSES)) {
        return ACK_STATUSES;
    }
};

/**
 * Converts a supported enum value into its protocol string.
 * Returns "unknown" if no matching entry exists.
 */
template <typename EnumType>
inline const char* toString(EnumType value) {
    for (const auto& entry : EnumTraits<EnumType>::entries()) {
        if (entry.value == value) {
            return entry.name;
        }
    }

    return "unknown";
}

/**
 * Converts a protocol string into its corresponding enum value.
 * Returns the caller-provided fallback when no match exists.
 */
template <typename EnumType>
inline EnumType fromString(
    const String& name,
    EnumType fallback
) {
    for (const auto& entry : EnumTraits<EnumType>::entries()) {
        if (name == entry.name) {
            return entry.value;
        }
    }

    return fallback;
}