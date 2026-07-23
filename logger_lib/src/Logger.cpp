#include "Logger.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace logging {

std::string toString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:
            return "DEBUG";
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Error:
            return "ERROR";
    }
    return "UNKNOWN";
}

Logger::Logger(const std::string& filename, LogLevel defaultLevel)
    : filename_(filename), currentLevel_(defaultLevel) {
    file_.open(filename_, std::ios::out | std::ios::app);
    if (!file_.is_open()) {
        throw std::runtime_error("Logger: не удалось открыть файл журнала: " +
                                  filename_);
    }
}

Logger::~Logger() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_.is_open()) {
        file_.close();
    }
}

std::string Logger::formatRecord(const std::string& message,
                                  LogLevel level) const {
    const auto now = std::chrono::system_clock::now();
    const std::time_t timeNow = std::chrono::system_clock::to_time_t(now);

    // localtime не потокобезопасен, поэтому localtime_r/localtime_s
    std::tm localTime{};
#if defined(_WIN32)
    localtime_s(&localTime, &timeNow);
#else
    localtime_r(&timeNow, &localTime);
#endif

    std::ostringstream oss;
    oss << '[' << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S") << "] "
        << '[' << toString(level) << "] " << message;

    return oss.str();
}

void Logger::log(const std::string& message, LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (level < currentLevel_) {
        return;
    }

    file_ << formatRecord(message, level) << std::endl;

    if (file_.fail()) {
        throw std::runtime_error("Logger: ошибка записи в файл журнала: " +
                                  filename_);
    }
}

void Logger::log(const std::string& message) {
    log(message, currentLevel_);
}

void Logger::setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    currentLevel_ = level;
}

LogLevel Logger::getLevel() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return currentLevel_;
}

} // namespace logging
