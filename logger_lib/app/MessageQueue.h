#ifndef LOGGER_APP_MESSAGE_QUEUE_H
#define LOGGER_APP_MESSAGE_QUEUE_H

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

// простая потокобезопасная очередь: один поток кладет, другой забирает
template <typename T>
class MessageQueue {
public:
    void push(T value) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(value));
        }
        cv_.notify_one();
    }

    // блокируется, пока не появится элемент или очередь не закроют
    std::optional<T> pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return !queue_.empty() || closed_; });

        if (queue_.empty()) {
            return std::nullopt;
        }

        T value = std::move(queue_.front());
        queue_.pop();
        return value;
    }

    void close() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            closed_ = true;
        }
        cv_.notify_all();
    }

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool closed_ = false;
};

#endif // LOGGER_APP_MESSAGE_QUEUE_H
