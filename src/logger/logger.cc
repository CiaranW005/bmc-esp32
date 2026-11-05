#include "logger/logger.h"
#include "logger/queue.h"

#include "network/manager.h"

#include "config/secrets.h"

#include <Arduino.h>

LogQueue Logger::queue_{};
UdpLogTransport Logger::udpTransport_{};
SerialLogTransport Logger::serialTransport_{};

void Logger::begin(const bmc::NetworkManager& manager) {
    static std::once_flag initFlag;

    std::call_once(initFlag, [](const bmc::NetworkManager& manager) {
        TaskHandle_t handle = nullptr;

        udpTransport_.setNetworkManager(manager);

        if (!udpTransport_.begin(LOG_HOST, LOG_PORT)) {
            Serial.println("[LOGGER] UDP transport initialisation failed");
            return;
        }

        if (xTaskCreate(
            loggingTask,
            "Logger",
            4096,
            nullptr,
            1,
            &handle
        ) == pdPASS) {
            queue_.setTaskHandle(handle);
        } else {
            Serial.println("[LOGGER] Failed to create logging task");
        }
    },  std::cref(manager));
}

void Logger::log(LogLevel level, const char* message) {
    LogEntry entry{};

    entry.level = level;
    entry.timestamp = millis();

    snprintf(entry.component, sizeof(entry.component), "%s", component_);
    snprintf(entry.message, sizeof(entry.message), "%s", message);

    queue_.push(entry);
}

void Logger::info(const char* message) {
   log(LogLevel::INFO, message);
}

void Logger::error(const char* message) {
    log(LogLevel::ERROR, message);
}

void Logger::loggingTask(void* parameter) {
    // FreeRTOS requires this parameter; unused because the task accesses static logger state.
    while (true) {
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000));

        LogEntry entry;

        while (queue_.peek(entry)) {
            bool sent = false;

            if (serialTransport_.available()) {
                sent |= serialTransport_.send(entry);
            }

            if (udpTransport_.available()) {
                sent |= udpTransport_.send(entry);
            }

            // Retain the entry if neither transport succeeds.
            // Success from either transport is sufficient to remove it from the queue.
            if (!sent) break;

            queue_.pop(entry.id);
        }
    }
}
