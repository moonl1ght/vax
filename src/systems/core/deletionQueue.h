#pragma once

#include "logger.h"
#include <deque>
#include <functional>

namespace vax::core {
class DeletionQueue {
  public:
    std::deque<std::function<void()>> deletors;

    void push_function(std::function<void()>&& function) { deletors.push_back(function); }

    void flush() {
        for (auto it = deletors.rbegin(); it != deletors.rend(); ++it) {
            (*it)();
        }

        deletors.clear();
        _logger.info("Flushed deletion queue");
    }

  private:
    Logger _logger = Logger("DeletionQueue");
};
} // namespace vax::core