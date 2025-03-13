# Error Logging Framework

A proof of concept error logging framework for the Channel Manager Architecture project using spdlog.

## Overview

This framework provides:

1. **Structured Error Codes**: Categorized error codes for different components
2. **Custom Exception Classes**: Hierarchy of exception classes for different error types
3. **Logging Functionality**: Flexible logging with multiple output options
4. **Integration Helpers**: Macros for easy integration into existing code

## Components

### Error Codes

The `ErrorCodes.h` file defines an enumeration of error codes categorized by component:

- System errors (1000-1999)
- Channel errors (2000-2999)
- Battery errors (3000-3999)
- Task errors (4000-4999)
- Generic errors (9000-9999)

Each error code has a string representation provided by the `ErrorCodeToString` function.

### Exception Classes

The `Exceptions.h` file defines a hierarchy of exception classes:

- `Exception`: Base class for all custom exceptions
- `SystemException`: For system-level errors
- `ChannelException`: For channel-related errors
- `BatteryException`: For battery-related errors
- `TaskException`: For task-related errors

Each exception stores:
- Error code
- Error message
- File name and line number (optional)

### Logger

The `Logger.h` and `Logger.cpp` files implement a singleton logger class that wraps spdlog functionality:

- Multiple log levels (TRACE, DEBUG, INFO, WARNING, ERROR, CRITICAL)
- Multiple sink types (console, file, rotating file, daily file)
- Error code integration
- Exception logging

## Usage

### Basic Logging

```cpp
#include "ErrorLogging/Logger.h"

// Initialize the logger
auto& logger = ErrorLogging::Logger::getInstance();
logger.initialize("channel_manager");
logger.addConsoleSink();
logger.addFileSink("channel_manager.log");

// Set log level
logger.setLogLevel(ErrorLogging::LogLevel::DEBUG);

// Log messages using macros
LOG_INFO("Application started");
LOG_DEBUG("Debug information");
LOG_WARNING("Warning message");
LOG_ERROR("Error message");
LOG_CRITICAL("Critical error");
```

### Logging with Error Codes

```cpp
LOG_ERROR_CODE(ErrorLogging::LogLevel::ERROR, 
              ErrorLogging::ErrorCode::CHANNEL_NOT_FOUND,
              "Failed to find channel with ID: CH001");
```

### Throwing and Catching Exceptions

```cpp
try {
    // Throw an exception with file and line information
    THROW_EXCEPTION(ErrorLogging::ChannelException, 
                   ErrorLogging::ErrorCode::CHANNEL_NOT_FOUND,
                   "Channel ID cannot be empty");
} catch (const ErrorLogging::Exception& e) {
    // Log the exception
    LOG_EXCEPTION(ErrorLogging::LogLevel::ERROR, e);
    
    // Handle specific exception types
    if (dynamic_cast<const ErrorLogging::ChannelException*>(&e)) {
        // Handle channel exception
    }
}
```

### Using the TRY_LOG_CATCH Macro

```cpp
TRY_LOG_CATCH({
    // Code that might throw an exception
    connectToChannel("CHAN001");
});
```

## Integration into Existing Project

To integrate this framework into the existing project:

1. Include the necessary headers in your source files:
   ```cpp
   #include "ErrorLogging/ErrorCodes.h"
   #include "ErrorLogging/Exceptions.h"
   #include "ErrorLogging/Logger.h"
   ```

2. Initialize the logger in your main function or application startup:
   ```cpp
   auto& logger = ErrorLogging::Logger::getInstance();
   logger.initialize("channel_manager");
   logger.addConsoleSink();
   logger.addFileSink("logs/channel_manager.log");
   logger.setLogLevel(ErrorLogging::LogLevel::INFO);
   ```

3. Replace existing error handling with the new exception classes:
   ```cpp
   // Before:
   if (channelId.empty()) {
       std::cerr << "Error: Channel ID cannot be empty" << std::endl;
       return false;
   }
   
   // After:
   if (channelId.empty()) {
       THROW_EXCEPTION(ErrorLogging::ChannelException, 
                      ErrorLogging::ErrorCode::CHANNEL_NOT_FOUND,
                      "Channel ID cannot be empty");
   }
   ```

4. Update catch blocks to log exceptions:
   ```cpp
   try {
       // Existing code
   } catch (const ErrorLogging::Exception& e) {
       LOG_EXCEPTION(ErrorLogging::LogLevel::ERROR, e);
       // Handle the error
   } catch (const std::exception& e) {
       LOG_EXCEPTION(ErrorLogging::LogLevel::ERROR, e);
       // Handle standard exceptions
   }
   ```

5. Add logging statements at key points in your code:
   ```cpp
   LOG_INFO("Starting battery test for battery: " + batteryId);
   // Perform battery test
   LOG_INFO("Battery test completed successfully");
   ```

## Dependencies

This framework depends on the [spdlog](https://github.com/gabime/spdlog) library for the actual logging functionality.

## Example

See `Example.cpp` for a complete example of how to use this framework.