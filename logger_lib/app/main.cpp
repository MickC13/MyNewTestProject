#include <cctype>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <thread>

#include "Logger.h"
#include "MessageQueue.h"

namespace {

struct LogTask {
    std::string message;
    logging::LogLevel level;
    bool hasLevel;
};

std::string toLower(std::string s) {
    for (auto& c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

std::optional<logging::LogLevel> parseLevel(const std::string& word) {
    const std::string lower = toLower(word);
    if (lower == "debug") return logging::LogLevel::Debug;
    if (lower == "info") return logging::LogLevel::Info;
    if (lower == "error") return logging::LogLevel::Error;
    return std::nullopt;
}

// строка вида "info текст сообщения" или просто "текст сообщения"
LogTask parseLine(const std::string& line) {
    std::istringstream iss(line);
    std::string firstWord;
    iss >> firstWord;

    if (auto level = parseLevel(firstWord)) {
        std::string rest;
        std::getline(iss, rest);
        const size_t start = rest.find_first_not_of(' ');
        std::string message = (start == std::string::npos) ? "" : rest.substr(start);
        return LogTask{message, *level, true};
    }

    return LogTask{line, logging::LogLevel::Info, false};
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Использование: " << argv[0]
                  << " <файл_журнала> [debug|info|error]" << std::endl;
        return 1;
    }

    const std::string logFile = argv[1];
    logging::LogLevel defaultLevel = logging::LogLevel::Info;

    if (argc >= 3) {
        auto level = parseLevel(argv[2]);
        if (!level) {
            std::cerr << "Неизвестный уровень важности: " << argv[2] << std::endl;
            return 1;
        }
        defaultLevel = *level;
    }

    std::unique_ptr<logging::Logger> logger;
    try {
        logger = std::make_unique<logging::Logger>(logFile, defaultLevel);
    } catch (const std::exception& e) {
        std::cerr << "Не удалось открыть журнал: " << e.what() << std::endl;
        return 1;
    }

    MessageQueue<LogTask> queue;

    // отдельный поток забирает сообщения из очереди и пишет их в файл,
    // пока ввод с консоли читает основной поток
    std::thread writer([&logger, &queue] {
        while (auto task = queue.pop()) {
            try {
                if (task->hasLevel) {
                    logger->log(task->message, task->level);
                } else {
                    logger->log(task->message);
                }
            } catch (const std::exception& e) {
                std::cerr << "Ошибка записи в журнал: " << e.what() << std::endl;
            }
        }
    });

    std::cout << "Журнал: " << logFile
              << ", уровень по умолчанию: " << logging::toString(defaultLevel)
              << std::endl;
    std::cout << "Формат ввода: \"[debug|info|error] текст сообщения\", "
                 "уровень можно не указывать."
              << std::endl;
    std::cout << "Для выхода: exit или Ctrl+D." << std::endl;

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "exit" || line == "quit") {
            break;
        }
        if (line.empty()) {
            continue;
        }
        queue.push(parseLine(line));
    }

    queue.close();
    writer.join();

    std::cout << "Завершение работы." << std::endl;
    return 0;
}
