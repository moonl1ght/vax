#pragma once

#include "notificationCenter.h"
#include <functional>
#include <mutex>
#include <vector>

namespace vax::core {
class ThreadRunner final {
  public:
    ThreadRunner() = default;
    ~ThreadRunner() = default;

    ThreadRunner(const ThreadRunner& other) = delete;
    ThreadRunner& operator=(const ThreadRunner& other) = delete;
    ThreadRunner(ThreadRunner&& other) noexcept = delete;
    ThreadRunner& operator=(ThreadRunner&& other) noexcept = delete;

    void runOnThread(std::function<void()> callback) {
        {
            std::lock_guard<std::mutex> lock(_queueMutex);
            _mainThreadQueue.push_back(callback);
        }
        vax::NotificationCenter::getInstance().wakeUpSDL();
    }

    void processThreadQueue() {
        std::vector<std::function<void()>> callbacksToExecute;

        {
            std::lock_guard<std::mutex> lock(_queueMutex);
            callbacksToExecute.swap(_mainThreadQueue);
        }

        for (const auto& callback : callbacksToExecute) {
            if (callback)
                callback();
        }
    }

  private:
    std::vector<std::function<void()>> _mainThreadQueue;
    std::mutex _queueMutex;
};
} // namespace vax::core