
#include <Arduino.h>

#include "logger/transporter/serial.h"

bool SerialLogTransport::available() const {
    return static_cast<bool>(Serial);
}

bool SerialLogTransport::send(const LogEntry& entry) {
    if (!available()) return false;


    Serial.printf(
        "[%lu][%s][%s] %s\r\n",
        static_cast<unsigned long>(entry.timestamp),
        entry.component,
        entry.level == LogLevel::ERROR ? "ERROR" : "INFO",
        entry.message
    );

    return true;
}