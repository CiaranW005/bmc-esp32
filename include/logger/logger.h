#pragma once

class Logger {
public:
    Logger(const char* component);

    void info(const char* message);
    void error(const char* message);

private:
    const char* component_;
};