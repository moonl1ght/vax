#pragma once

#include "logger.h"
#include "physicalCore.h"
#include <optional>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <pthread.h>
#include <sched.h>
#include <sys/resource.h>
#endif

namespace vax::core {

class Thread final {
  public:
    enum class Priority { Normal, Medium, High };

    friend std::ostream& operator<<(std::ostream& os, Priority p) {
        switch (p) {
        case Priority::Normal:
            return os << "Normal";
        case Priority::Medium:
            return os << "Medium";
        case Priority::High:
            return os << "High";
        }
        return os << "Unknown";
    }

    explicit Thread() noexcept {};

    ~Thread() = default;

    Thread(const Thread& other) = delete;
    Thread& operator=(const Thread& other) = delete;
    Thread(Thread&& other) noexcept = default;
    Thread& operator=(Thread&& other) noexcept = default;

    template <typename Func> void start(Func&& func) noexcept {
        _jthread = std::jthread([this, func = std::forward<Func>(func)]() {
            if (_pinnedCore) {
                _pinToCore(_pinnedCore->primary_logical_id);
            }
            if (_priority != Priority::Normal && !_setCurrentThreadPriority(_priority)) {
                _logger.warning("Failed to set thread priority to ", _priority);
            }
            func();
        });
    }

    void setPinnedCore(std::optional<PhysicalCore> core) noexcept { _pinnedCore = core; }

    void setPriority(Priority priority) noexcept { _priority = priority; }

  private:
    dev::Logger _logger = dev::Logger("Thread");

    std::jthread _jthread;

    std::optional<PhysicalCore> _pinnedCore;

    Priority _priority = Priority::Normal;

    bool _pinToCore(unsigned int logical_core_id) noexcept {
#ifdef _WIN32
        DWORD_PTR mask = (static_cast<DWORD_PTR>(1) << logical_core_id);
        DWORD_PTR prev_mask = SetThreadAffinityMask(GetCurrentThread(), mask);
        if (prev_mask == 0) {
            return false;
        }
        return GetCurrentProcessorNumber() == logical_core_id;

#elif __linux__
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(logical_core_id, &cpuset);

        int rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
        if (rc != 0) {
            return false;
        }
        return sched_getcpu() == static_cast<int>(logical_core_id);
#else
        return false;
#endif
    }

    bool _setCurrentThreadPriority(Priority priority) noexcept {
#ifdef _WIN32
        int win_prio = THREAD_PRIORITY_NORMAL;
        switch (priority) {
        case Priority::Normal:
            win_prio = THREAD_PRIORITY_NORMAL;
            break;
        case Priority::AboveNormal:
            win_prio = THREAD_PRIORITY_ABOVE_NORMAL;
            break;
        case Priority::High:
            win_prio = THREAD_PRIORITY_HIGHEST;
            break;
        }
        return SetThreadPriority(GetCurrentThread(), win_prio) != 0;

#elif __linux__
        int nice_val = 0;
        switch (priority) {
        case Priority::Normal:
            nice_val = 0;
            break;
        case Priority::Medium:
            nice_val = -5;
            break;
        case Priority::High:
            nice_val = -10;
            break;
        }
        return setpriority(PRIO_PROCESS, 0, nice_val) == 0;
#else
        return false;
#endif
    }
};

} // namespace vax::core