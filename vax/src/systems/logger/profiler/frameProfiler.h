#pragma once

#include "profiler.h"

namespace vax {
class FrameProfiler final : public Profiler {
  public:
    struct FrameInfo {
        std::chrono::steady_clock::duration duration = std::chrono::steady_clock::duration::zero();
        int fps = 0;
    };

    FrameProfiler()
        : Profiler() {}

    ~FrameProfiler() = default;

    void beginFrameZone(const std::string& name);

    FrameInfo endFrameZone(const std::string& name);

    std::optional<FrameInfo> getFrameZoneInfo(const std::string& name) const;

  private:
    std::unordered_map<std::string, FrameInfo> _frameZoneInfos;
};
} // namespace vax