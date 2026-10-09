#pragma once

#include "debouncer.h"
#include "frameProfiler.h"
#include "systemInfo.h"
#include "view.h"

namespace vax::ui {
class StatsView final : public View {
  public:
    StatsView(vax::engine::Renderer& renderer)
        : View(renderer)
        , _debouncer(std::chrono::milliseconds(500)) {
        _systemInfo = SystemInfo();
    };

    ~StatsView() = default;

    void update(const vax::engine::FrameTime& frameTime) override;

    void linkFrameProfiler(vax::FrameProfiler* frameProfiler) noexcept {
        _frameProfiler = frameProfiler;
    }

  private:
    SystemInfo _systemInfo;
    FrameProfiler* _frameProfiler;
    vax::core::Debouncer _debouncer;
    SystemInfo::MemoryStats _memoryStats = SystemInfo::MemoryStats();
    SystemInfo::GPUStats _gpuStats = SystemInfo::GPUStats();
    FrameProfiler::FrameInfo _frameInfo = FrameProfiler::FrameInfo();
    FrameProfiler::FrameInfo _frameWorkloadInfo = FrameProfiler::FrameInfo();

    void _showSystemInfo() const;

    void _showFrameInfo() const;

    void _updateStats();
};
} // namespace vax::ui