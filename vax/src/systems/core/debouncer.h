#pragma once

#include <chrono>

namespace vax::core {
class Debouncer {
  public:
    Debouncer(std::chrono::milliseconds interval)
        : _interval(interval) {}

    template <typename Func> void execute(Func&& func) {
        if (_shouldExecute()) {
            func();
        }
    }

  private:
    std::chrono::milliseconds _interval;
    std::chrono::steady_clock::time_point _lastTime;

    bool _shouldExecute() {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastTime);
        if (elapsed.count() >= _interval.count()) {
            _lastTime = now;
            return true;
        }
        return false;
    }
};
} // namespace vax::core