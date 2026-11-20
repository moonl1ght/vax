#pragma once
#include <chrono>
#include <string>
#include <tracy/Tracy.hpp>
#include <unordered_map>

namespace vax {
class Profiler {
  public:
    Profiler() = default;
    virtual ~Profiler() = default;

    Profiler(const Profiler&) = delete;
    Profiler(Profiler&&) noexcept = default;
    Profiler& operator=(const Profiler&) = delete;
    Profiler& operator=(Profiler&&) noexcept = default;

    void begin(const std::string& name);

    std::chrono::steady_clock::duration end(const std::string& name);

  protected:
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> _startTimes;
    std::unordered_map<std::string, std::chrono::steady_clock::duration> _durations;
};
} // namespace vax