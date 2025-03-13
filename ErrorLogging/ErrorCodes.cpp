#include "ErrorCodes.h"

namespace ErrorLogging {

std::string ErrorCodeToString(ErrorCode code) {
    switch (code) {
        // System level errors
        case ErrorCode::SYSTEM_ERROR:
            return "System error";
        case ErrorCode::MEMORY_ALLOCATION_FAILED:
            return "Memory allocation failed";
        case ErrorCode::FILE_NOT_FOUND:
            return "File not found";
        case ErrorCode::PERMISSION_DENIED:
            return "Permission denied";
            
        // Channel manager errors
        case ErrorCode::CHANNEL_ERROR:
            return "Channel error";
        case ErrorCode::CHANNEL_NOT_FOUND:
            return "Channel not found";
        case ErrorCode::CHANNEL_ALREADY_EXISTS:
            return "Channel already exists";
        case ErrorCode::CHANNEL_INITIALIZATION_FAILED:
            return "Channel initialization failed";
        case ErrorCode::CHANNEL_COMMUNICATION_ERROR:
            return "Channel communication error";
            
        // Battery testing errors
        case ErrorCode::BATTERY_ERROR:
            return "Battery error";
        case ErrorCode::BATTERY_TEST_FAILED:
            return "Battery test failed";
        case ErrorCode::BATTERY_OVERHEATING:
            return "Battery overheating";
        case ErrorCode::BATTERY_VOLTAGE_OUT_OF_RANGE:
            return "Battery voltage out of range";
        case ErrorCode::BATTERY_CURRENT_OUT_OF_RANGE:
            return "Battery current out of range";
            
        // Task errors
        case ErrorCode::TASK_ERROR:
            return "Task error";
        case ErrorCode::TASK_CREATION_FAILED:
            return "Task creation failed";
        case ErrorCode::TASK_EXECUTION_FAILED:
            return "Task execution failed";
        case ErrorCode::TASK_TIMEOUT:
            return "Task timeout";
        case ErrorCode::TASK_INVALID_PARAMETERS:
            return "Task invalid parameters";
            
        // Generic errors
        case ErrorCode::UNKNOWN_ERROR:
            return "Unknown error";
        case ErrorCode::INVALID_ARGUMENT:
            return "Invalid argument";
        case ErrorCode::NOT_IMPLEMENTED:
            return "Not implemented";
        case ErrorCode::OPERATION_TIMEOUT:
            return "Operation timeout";
            
        default:
            return "Undefined error";
    }
}

} // namespace ErrorLogging