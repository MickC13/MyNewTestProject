#ifndef LOGGER_LIBRARY_LOGGER_H
#define LOGGER_LIBRARY_LOGGER_H

#include <fstream>
#include <mutex>
#include <string>

namespace logging {

// Debug < Info < Error по важности
enum class LogLevel : int {
    Debug = 0,
    Info = 1,
    Error = 2
};

std::string toString(LogLevel level);

class Logger {
public:
    // filename - путь до файла журнала, открывается на дозапись
    // defaultLevel - сообщения ниже этого уровня отбрасываются
    explicit Logger(const std::string& filename,
                     LogLevel defaultLevel = LogLevel::Info);

    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    void log(const std::string& message, LogLevel level);
    void log(const std::string& message); // с текущим уровнем по умолчанию

    void setLevel(LogLevel level);
    LogLevel getLevel() const;

private:
    std::string formatRecord(const std::string& message, LogLevel level) const;

    std::string filename_;
    std::ofstream file_;
    LogLevel currentLevel_;
    mutable std::mutex mutex_;
};

} // namespace logging

#endif // LOGGER_LIBRARY_LOGGER_H
