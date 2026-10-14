
#include "logger/queue.h"
#include <task.h>

#include <Arduino.h>

LogQueue::LogQueue() {
    for (int i = 0; i < CAPACITY - 1; ++i) {
        nodes_[i].next = i + 1;
    }

    nodes_[CAPACITY - 1].next = -1;
}

bool LogQueue::push(LogEntry& entry) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (freeHead_ == -1) {
            return false;
        }

        int idx = freeHead_;
        freeHead_ = nodes_[idx].next;

        entry.id = nextId_++;
        nodes_[idx].entry = entry;
        nodes_[idx].next = -1;

        if (tail_ == -1) {
            head_ = idx;
        } else {
            nodes_[tail_].next = idx;
        }

        tail_ = idx;
        ++count_;
    }

    notify();
    return true;
}

bool LogQueue::peek(LogEntry& entry) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (head_ == -1) return false;

    entry = nodes_[head_].entry;
    return true;
}

bool LogQueue::pop(uint64_t expectedId) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (head_ == -1) return false;

    if (nodes_[head_].entry.id != expectedId) {
        return false;
    }

    int idx = head_;
    head_ = nodes_[idx].next;

    if (head_ == -1) tail_ = -1;

    nodes_[idx].next = freeHead_;
    freeHead_ = idx;

    --count_;
    return true;
}

void LogQueue::setTaskHandle(TaskHandle_t handle) {
    loggingTaskHandle_ = handle;
}

LogQueue::NodePosition LogQueue::findOldest(LogLevel level) const {
    int prev = -1;
    int current = head_;

    while (current != -1) {
        if (nodes_[current].entry.level == level) {
            return {current, prev};
        }

        prev = current;
        current = nodes_[current].next;
    }

    return {-1, -1};
}

void LogQueue::removeAt(LogQueue::NodePosition pos) {
    if (pos.current == -1) {
        return;
    }

    if (pos.prev == -1) {
        head_ = nodes_[pos.current].next;
    } else {
        nodes_[pos.prev].next = nodes_[pos.current].next;
    }

    if (tail_ == pos.current) {
        tail_ = pos.prev;
    }

    nodes_[pos.current].next = freeHead_;
    freeHead_ = pos.current;

    --count_;
}


void LogQueue::notify() {
    if (loggingTaskHandle_ != nullptr) {
        xTaskNotifyGive(loggingTaskHandle_);
    }
}