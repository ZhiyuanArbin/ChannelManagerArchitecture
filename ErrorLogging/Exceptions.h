#pragma once

#include <exception>
#include <string>
#include <sstream>
#include "ErrorCodes.h"

namespace ErrorLogging {

// Base exception class for all custom exceptions
class Exception : public std::exception {
public:
    Exception(ErrorCode code, const std::string& message, const std::string& file = "", int line = 0)
        : m_errorCode(code), m_message(message), m_file(file), m_line(line) {
        formatFullMessage();
    }

    virtual ~Exception() noexcept = default;

    // Override std::exception::what()
    const char* what() const noexcept override {
        return m_fullMessage.c_str();
    }

    // Getters
    ErrorCode getErrorCode() const { return m_errorCode; }
    const std::string& getMessage() const { return m_message; }
    const std::string& getFile() const { return m_file; }
    int getLine() const { return m_line; }

private:
    void formatFullMessage() {
        std::ostringstream oss;
        oss << "Error " << static_cast<int>(m_errorCode) << " (" << ErrorCodeToString(m_errorCode) << "): " 
            << m_message;
        
        if (!m_file.empty()) {
            oss << " [" << m_file;
            if (m_line > 0) {
                oss << ":" << m_line;
            }
            oss << "]";
        }
        
        m_fullMessage = oss.str();
    }

    ErrorCode m_errorCode;
    std::string m_message;
    std::string m_file;
    int m_line;
    std::string m_fullMessage;
};

// System exceptions
class SystemException : public Exception {
public:
    SystemException(ErrorCode code, const std::string& message, const std::string& file = "", int line = 0)
        : Exception(code, message, file, line) {
        if (static_cast<int>(code) < 1000 || static_cast<int>(code) >= 2000) {
            throw std::invalid_argument("SystemException requires a SYSTEM error code (1000-1999)");
        }
    }
};

// Channel exceptions
class ChannelException : public Exception {
public:
    ChannelException(ErrorCode code, const std::string& message, const std::string& file = "", int line = 0)
        : Exception(code, message, file, line) {
        if (static_cast<int>(code) < 2000 || static_cast<int>(code) >= 3000) {
            throw std::invalid_argument("ChannelException requires a CHANNEL error code (2000-2999)");
        }
    }
};

// Battery exceptions
class BatteryException : public Exception {
public:
    BatteryException(ErrorCode code, const std::string& message, const std::string& file = "", int line = 0)
        : Exception(code, message, file, line) {
        if (static_cast<int>(code) < 3000 || static_cast<int>(code) >= 4000) {
            throw std::invalid_argument("BatteryException requires a BATTERY error code (3000-3999)");
        }
    }
};

// Task exceptions
class TaskException : public Exception {
public:
    TaskException(ErrorCode code, const std::string& message, const std::string& file = "", int line = 0)
        : Exception(code, message, file, line) {
        if (static_cast<int>(code) < 4000 || static_cast<int>(code) >= 5000) {
            throw std::invalid_argument("TaskException requires a TASK error code (4000-4999)");
        }
    }
};

// Macro to throw exceptions with file and line information
#define THROW_EXCEPTION(ExceptionType, ErrorCode, Message) \
    throw ExceptionType(ErrorCode, Message, __FILE__, __LINE__)

} // namespace ErrorLogging