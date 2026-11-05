#pragma once

#include <stdint.h>
#include <cstddef>
#include <mutex>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

enum class LogLevel : uint8_t {
    INFO,
    ERROR
};

struct LogEntry {
    uint64_t id;
    LogLevel level;
    char component[16];
    char message[128];
    uint32_t timestamp;
};

class LogQueue {
public:
    LogQueue();

    bool push(LogEntry& entry);
    bool peek(LogEntry& entry);

    bool pop(uint64_t expectedID);

    void setTaskHandle(TaskHandle_t handle);
    void initialise();

private:
    static constexpr size_t CAPACITY = 64;

    struct Node {
        LogEntry entry;
        int next = -1;
    };

    struct NodePosition {
        int current = -1;
        int prev = -1;
    };

    // Fixed node pool shared by the queued-entry list and the free-slot list.
    // Links are array indices; -1 marks the end of either list.
    Node nodes_[CAPACITY];

    int head_ = -1;
    int tail_ = -1;
    int freeHead_ = 0;

    size_t count_ = 0;
    uint64_t nextId_ = 0;

    std::mutex mutex_;

    TaskHandle_t loggingTaskHandle_ = nullptr;

    NodePosition findOldest(LogLevel level) const;
    void removeAt(NodePosition);

    void notify();
};