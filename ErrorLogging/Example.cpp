#include "ErrorCodes.h"
#include "Exceptions.h"
#include "Logger.h"

#include <iostream>
#include <string>

// This is a proof of concept example showing how to use the error logging framework

// Example function that might throw an exception
void connectToChannel(const std::string& channelId) {
    // Simulate a channel not found error
    if (channelId.empty()) {
        // Using the macro to throw an exception with file and line information
        THROW_EXCEPTION(ErrorLogging::ChannelException, 
                       ErrorLogging::ErrorCode::CHANNEL_NOT_FOUND,
                       "Channel ID cannot be empty");
    }
    
    // Simulate a channel initialization error
    if (channelId == "invalid") {
        THROW_EXCEPTION(ErrorLogging::ChannelException,
                       ErrorLogging::ErrorCode::CHANNEL_INITIALIZATION_FAILED,
                       "Failed to initialize channel with ID: " + channelId);
    }
    
    // Success case
    std::cout << "Successfully connected to channel: " << channelId << std::endl;
}

// Example function for battery testing
void testVoltage(const std::string& batteryId, double voltage) {
    // Simulate a voltage out of range error
    if (voltage < 3.0 || voltage > 4.2) {
        THROW_EXCEPTION(ErrorLogging::BatteryException,
                       ErrorLogging::ErrorCode::BATTERY_VOLTAGE_OUT_OF_RANGE,
                       "Battery voltage out of range: " + std::to_string(voltage) + "V");
    }
    
    // Success case
    std::cout << "Battery " << batteryId << " tested successfully at " << voltage << "V" << std::endl;
}

// Example function for task execution
bool executeTask(const std::string& taskId, int timeoutMs) {
    // Simulate a timeout error
    if (timeoutMs < 100) {
        THROW_EXCEPTION(ErrorLogging::TaskException,
                       ErrorLogging::ErrorCode::TASK_TIMEOUT,
                       "Task timeout too short: " + std::to_string(timeoutMs) + "ms");
    }
    
    // Simulate an invalid parameter error
    if (taskId.empty()) {
        THROW_EXCEPTION(ErrorLogging::TaskException,
                       ErrorLogging::ErrorCode::TASK_INVALID_PARAMETERS,
                       "Task ID cannot be empty");
    }
    
    // Success case
    std::cout << "Task " << taskId << " executed successfully" << std::endl;
    return true;
}

int main() {
    try {
        // Initialize the logger
        auto& logger = ErrorLogging::Logger::getInstance();
        logger.initialize("channel_manager");
        
        // Add console sink for logging to console
        logger.addConsoleSink();
        
        // Add file sink for logging to file
        logger.addFileSink("channel_manager.log");
        
        // Set global log level
        logger.setLogLevel(ErrorLogging::LogLevel::DEBUG);
        
        // Log some messages
        LOG_INFO("Application started");
        LOG_DEBUG("Debug information");
        
        // Example of using error codes in logs
        LOG_ERROR_CODE(ErrorLogging::LogLevel::WARNING, 
                      ErrorLogging::ErrorCode::SYSTEM_ERROR,
                      "This is a system error example");
        
        // Example of using try-catch with logging
        try {
            // Try to connect to a channel
            connectToChannel("");  // This will throw an exception
        } catch (const ErrorLogging::Exception& e) {
            // Log the exception
            LOG_EXCEPTION(ErrorLogging::LogLevel::ERROR, e);
            
            // We can also handle specific exception types
            if (dynamic_cast<const ErrorLogging::ChannelException*>(&e)) {
                std::cout << "Caught a channel exception: " << e.what() << std::endl;
            }
        }
        
        // Example of using the TRY_LOG_CATCH macro
        TRY_LOG_CATCH({
            // Try to test a battery with invalid voltage
            testBattery("BAT001", 5.0);  // This will throw an exception
        });
        
        // Example of successful operations
        TRY_LOG_CATCH({
            connectToChannel("CHAN001");  // This should succeed
            testVoltage("BAT002", 3.7);   // This should succeed
            executeTask("TASK001", 1000); // This should succeed
        });
        
        // Example of multiple exceptions
        try {
            // Try to execute a task with invalid parameters
            executeTask("", 50);  // This will throw an exception
        } catch (const ErrorLogging::TaskException& e) {
            LOG_EXCEPTION(ErrorLogging::LogLevel::ERROR, e);
            std::cout << "Task exception caught: " << e.what() << std::endl;
        } catch (const std::exception& e) {
            LOG_EXCEPTION(ErrorLogging::LogLevel::ERROR, e);
            std::cout << "Standard exception caught: " << e.what() << std::endl;
        }
        
        // Flush logs before exit
        logger.flush();
        LOG_INFO("Application shutting down");
        
    } catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}