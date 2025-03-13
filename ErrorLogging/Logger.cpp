#include "Logger.h"
#include "Exceptions.h"

// Include spdlog headers
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>

namespace ErrorLogging {

Logger::Logger() : m_initialized(false) {
    // Constructor is private due to singleton pattern
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::initialize(const std::string& loggerName) {
    if (m_initialized) {
        return;
    }
    
    // Create logger
    m_logger = std::make_shared<spdlog::logger>(loggerName);
    
    // Set pattern
    m_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [thread %t] %v");
    
    // Register with spdlog
    spdlog::register_logger(m_logger);
    
    m_initialized = true;
}

void Logger::addFileSink(const std::string& filename, bool truncate) {
    if (!m_initialized) {
        initialize();
    }
    
    try {
        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, truncate);
        m_sinks.push_back(sink);
        m_logger->sinks().push_back(sink);
    }
    catch (const spdlog::spdlog_ex& ex) {
        throw SystemException(ErrorCode::FILE_NOT_FOUND, 
            std::string("Failed to create file sink: ") + ex.what());
    }
}

void Logger::addConsoleSink() {
    if (!m_initialized) {
        initialize();
    }
    
    auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    m_sinks.push_back(sink);
    m_logger->sinks().push_back(sink);
}

void Logger::addRotatingFileSink(const std::string& filename, size_t maxFileSize, size_t maxFiles) {
    if (!m_initialized) {
        initialize();
    }
    
    try {
        auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            filename, maxFileSize, maxFiles);
        m_sinks.push_back(sink);
        m_logger->sinks().push_back(sink);
    }
    catch (const spdlog::spdlog_ex& ex) {
        throw SystemException(ErrorCode::FILE_NOT_FOUND, 
            std::string("Failed to create rotating file sink: ") + ex.what());
    }
}

void Logger::addDailyFileSink(const std::string& filename, int hour, int minute) {
    if (!m_initialized) {
        initialize();
    }
    
    try {
        auto sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
            filename, hour, minute);
        m_sinks.push_back(sink);
        m_logger->sinks().push_back(sink);
    }
    catch (const spdlog::spdlog_ex& ex) {
        throw SystemException(ErrorCode::FILE_NOT_FOUND, 
            std::string("Failed to create daily file sink: ") + ex.what());
    }
}

void Logger::setLogLevel(LogLevel level) {
    if (!m_initialized) {
        initialize();
    }
    
    m_logger->set_level(static_cast<spdlog::level::level_enum>(toSpdlogLevel(level)));
}

void Logger::trace(const std::string& message) {
    if (!m_initialized) {
        initialize();
    }
    
    m_logger->trace(message);
}

void Logger::debug(const std::string& message) {
    if (!m_initialized) {
        initialize();
    }
    
    m_logger->debug(message);
}

void Logger::info(const std::string& message) {
    if (!m_initialized) {
        initialize();
    }
    
    m_logger->info(message);
}

void Logger::warning(const std::string& message) {
    if (!m_initialized) {
        initialize();
    }
    
    m_logger->warn(message);
}

void Logger::error(const std::string& message) {
    if (!m_initialized) {
        initialize();
    }
    
    m_logger->error(message);
}

void Logger::critical(const std::string& message) {
    if (!m_initialized) {
        initialize();
    }
    
    m_logger->critical(message);
}

void Logger::log(LogLevel level, ErrorCode code, const std::string& message) {
    if (!m_initialized) {
        initialize();
    }
    
    std::string formattedMessage = 
        "Error " + std::to_string(static_cast<int>(code)) + 
        " (" + ErrorCodeToString(code) + "): " + message;
    
    switch (level) {
        case LogLevel::TRACE:
            m_logger->trace(formattedMessage);
            break;
        case LogLevel::DEBUG:
            m_logger->debug(formattedMessage);
            break;
        case LogLevel::INFO:
            m_logger->info(formattedMessage);
            break;
        case LogLevel::WARNING:
            m_logger->warn(formattedMessage);
            break;
        case LogLevel::ERROR:
            m_logger->error(formattedMessage);
            break;
        case LogLevel::CRITICAL:
            m_logger->critical(formattedMessage);
            break;
        default:
            // Do nothing for OFF
            break;
    }
}

void Logger::logException(LogLevel level, const std::exception& exception) {
    if (!m_initialized) {
        initialize();
    }
    
    // Check if it's our custom exception
    const Exception* customEx = dynamic_cast<const Exception*>(&exception);
    if (customEx) {
        // Use error code from custom exception
        log(level, customEx->getErrorCode(), customEx->getMessage());
    } else {
        // Generic exception
        std::string message = "Exception: ";
        message += exception.what();
        
        switch (level) {
            case LogLevel::TRACE:
                m_logger->trace(message);
                break;
            case LogLevel::DEBUG:
                m_logger->debug(message);
                break;
            case LogLevel::INFO:
                m_logger->info(message);
                break;
            case LogLevel::WARNING:
                m_logger->warn(message);
                break;
            case LogLevel::ERROR:
                m_logger->error(message);
                break;
            case LogLevel::CRITICAL:
                m_logger->critical(message);
                break;
            default:
                // Do nothing for OFF
                break;
        }
    }
}

void Logger::flush() {
    if (!m_initialized) {
        initialize();
    }
    
    m_logger->flush();
}

int Logger::toSpdlogLevel(LogLevel level) {
    switch (level) {
        case LogLevel::TRACE:
            return spdlog::level::trace;
        case LogLevel::DEBUG:
            return spdlog::level::debug;
        case LogLevel::INFO:
            return spdlog::level::info;
        case LogLevel::WARNING:
            return spdlog::level::warn;
        case LogLevel::ERROR:
            return spdlog::level::err;
        case LogLevel::CRITICAL:
            return spdlog::level::critical;
        case LogLevel::OFF:
            return spdlog::level::off;
        default:
            return spdlog::level::info;
    }
}

} // namespace ErrorLogging