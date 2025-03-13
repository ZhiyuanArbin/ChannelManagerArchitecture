#pragma once

#include <string>

namespace ErrorLogging {

// Error code enumeration
enum class ErrorCode {
    // System level errors
    SYSTEM_ERROR = 1000,
    MEMORY_ALLOCATION_FAILED = 1001,
    FILE_NOT_FOUND = 1002,
    PERMISSION_DENIED = 1003,
    
    // Channel manager errors
    CHANNEL_ERROR = 2000,
    CHANNEL_NOT_FOUND = 2001,
    CHANNEL_ALREADY_EXISTS = 2002,
    CHANNEL_INITIALIZATION_FAILED = 2003,
    CHANNEL_COMMUNICATION_ERROR = 2004,
    
    // Battery testing errors
    BATTERY_ERROR = 3000,
    BATTERY_TEST_FAILED = 3001,
    BATTERY_OVERHEATING = 3002,
    BATTERY_VOLTAGE_OUT_OF_RANGE = 3003,
    BATTERY_CURRENT_OUT_OF_RANGE = 3004,
    
    // Task errors
    TASK_ERROR = 4000,
    TASK_CREATION_FAILED = 4001,
    TASK_EXECUTION_FAILED = 4002,
    TASK_TIMEOUT = 4003,
    TASK_INVALID_PARAMETERS = 4004,
    
    // Generic errors
    UNKNOWN_ERROR = 9000,
    INVALID_ARGUMENT = 9001,
    NOT_IMPLEMENTED = 9002,
    OPERATION_TIMEOUT = 9003
};

// Convert error code to string representation
std::string ErrorCodeToString(ErrorCode code);

} // namespace ErrorLogging