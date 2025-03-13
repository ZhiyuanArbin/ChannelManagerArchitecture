#pragma once

#include <string>
#include <memory>
#include <vector>
#include "ErrorCodes.h"

// Forward declarations for spdlog
namespace spdlog {
    class logger;
    
    namespace sinks {
        class sink;
    }
}

namespace ErrorLogging {

// Log levels
enum class LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL,
    OFF
};

// Logger class
class Logger {
public:
    // Get singleton instance
    static Logger& getInstance();
    
    // Initialize the logger with sinks
    void initialize(const std::string& loggerName = "channel_manager_logger");
    
    // Add a file sink
    void addFileSink(const std::string& filename, bool truncate = false);
    
    // Add a console sink
    void addConsoleSink();
    
    // Add a rotating file sink
    void addRotatingFileSink(const std::string& filename, size_t maxFileSize, size_t maxFiles);
    
    // Add a daily file sink
    void addDailyFileSink(const std::string& filename, int hour = 0, int minute = 0);
    
    // Set global log level
    void setLogLevel(LogLevel level);
    
    // Log methods
    void trace(const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void critical(const std::string& message);
    
    // Log with error code
    void log(LogLevel level, ErrorCode code, const std::string& message);
    
    // Log exception
    void logException(LogLevel level, const std::exception& exception);
    
    // Flush all sinks
    void flush();
    
private:
    // Private constructor for singleton
    Logger();
    
    // Deleted copy constructor and assignment operator
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    // Convert LogLevel to spdlog level
    int toSpdlogLevel(LogLevel level);
    
    // The actual spdlog logger
    std::shared_ptr<spdlog::logger> m_logger;
    
    // Collection of sinks
    std::vector<std::shared_ptr<spdlog::sinks::sink>> m_sinks;
    
    // Flag to check if logger is initialized
    bool m_initialized;
};

// Convenience macros for logging
#define LOG_TRACE(message) ErrorLogging::Logger::getInstance().trace(message)
#define LOG_DEBUG(message) ErrorLogging::Logger::getInstance().debug(message)
#define LOG_INFO(message) ErrorLogging::Logger::getInstance().info(message)
#define LOG_WARNING(message) ErrorLogging::Logger::getInstance().warning(message)
#define LOG_ERROR(message) ErrorLogging::Logger::getInstance().error(message)
#define LOG_CRITICAL(message) ErrorLogging::Logger::getInstance().critical(message)

// Macros for logging with error code
#define LOG_ERROR_CODE(level, code, message) ErrorLogging::Logger::getInstance().log(level, code, message)

// Macro for logging exceptions
#define LOG_EXCEPTION(level, exception) ErrorLogging::Logger::getInstance().logException(level, exception)

// Macro for try-catch with logging
#define TRY_LOG_CATCH(code) \
    try { \
        code \
    } catch (const ErrorLogging::Exception& e) { \
        LOG_EXCEPTION(ErrorLogging::LogLevel::ERROR, e); \
    } catch (const std::exception& e) { \
        LOG_EXCEPTION(ErrorLogging::LogLevel::ERROR, e); \
    } catch (...) { \
        LOG_CRITICAL("Unknown exception caught"); \
    }

} // namespace ErrorLogging