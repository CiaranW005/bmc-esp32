#include "logger/logger.h"

#include <Arduino.h>

Logger::Logger(const char* component): component_(component) {}

void Logger::info(const char* message) {
    Serial.printf("[%s] %s\n", component_, message);
}

void Logger::error(const char* message) {
    Serial.printf("[%s][ERROR] %s\n", component_, message);
}
