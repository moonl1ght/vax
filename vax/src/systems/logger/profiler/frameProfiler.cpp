#include "frameProfiler.h"

using namespace vax;

void FrameProfiler::beginFrameZone(const std::string& name) { Profiler::begin(name); }

FrameProfiler::FrameInfo FrameProfiler::endFrameZone(const std::string& name) {
    auto duration = Profiler::end(name);
    auto duration_seconds = std::chrono::duration<float>(duration).count();
    FrameInfo frameInfo{duration, -1};
    if (duration_seconds > 0.0f) [[likely]] {
        frameInfo = FrameInfo{duration, static_cast<int>(1.0f / duration_seconds)};
    }
    _frameZoneInfos[name] = frameInfo;
    return frameInfo;
}

std::optional<FrameProfiler::FrameInfo> FrameProfiler::getFrameZoneInfo(const std::string& name) const {
    auto it = _frameZoneInfos.find(name);
    if (it == _frameZoneInfos.end()) {
        return std::nullopt;
    }
    return it->second;
}