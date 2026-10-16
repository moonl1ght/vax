#pragma once

#include <chrono>
#include <cstdint>

namespace vax::engine {
struct FrameTime final {
    std::chrono::nanoseconds timestamp = std::chrono::nanoseconds::zero();
    std::chrono::nanoseconds deltaTimePrecise = std::chrono::nanoseconds::zero();
    float deltaTime = 0.0f;
    float fps = 0.0f;
    uint64_t frameCount = 0;
};
} // namespace vax::engine