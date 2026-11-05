#pragma once

#include "logger/logger.h"

enum class ManagementResult {
    Success,
    Unavailable,
    Failed
};

/**
 * Defines the preferred order for management controllers.
 * Lower values represent less invasive methods.
 * Callers must implement any selection or fallback behavior.
 */
enum class ManagementPriority {
    Network = 100,
    Power = 200
};

/**
 * Common interface for PC management controllers.
 *
 * Implementations provide different mechanisms for performing
 * power operations, allowing the caller to select a controller
 * based on availability and priority.
 */
class ManagementController {
public:
    ManagementController(const char* component): logger_(component) {}
    
    virtual ManagementPriority priority() const = 0;
    
    virtual ManagementResult powerOn() = 0;
    virtual ManagementResult powerOff() = 0;
    virtual ManagementResult reset() = 0;

    virtual ~ManagementController() = default;

protected:
    Logger logger_;
};