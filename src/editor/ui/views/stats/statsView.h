#pragma once

#include "debouncer.h"
#include "frameProfiler.h"
#include "fullStatsView.h"
#include "renderer.h"
#include "systemInfo.h"
#include "view.h"

namespace vax::editor {
class StatsView final : public View {
  public:
    StatsView(vax::gfx::Renderer& renderer)
        : View(renderer)
        , _debouncer(std::chrono::milliseconds(500)) {
        _systemInfo = dev::SystemInfo();
        _fullStatsView = std::make_unique<FullStatsView>(renderer);
    };

    ~StatsView() = default;

    void update(const vax::gfx::FrameTime& frameTime) override {};

    void updateUI(const vax::gfx::FrameTime& frameTime) override;

    void linkFrameProfiler(dev::FrameProfiler* frameProfiler) noexcept { _frameProfiler = frameProfiler; }

    bool isShown() const noexcept { return _isShown; }

    void setShown(bool isShown) noexcept { _isShown = isShown; }

  private:
    static constexpr ImVec2 SIZE = ImVec2(600.0f, 300.0f);

    bool _isShown = false;
    std::unique_ptr<FullStatsView> _fullStatsView;
    dev::SystemInfo _systemInfo;
    dev::FrameProfiler* _frameProfiler = nullptr;
    vax::core::Debouncer _debouncer;
    dev::SystemInfo::MemoryStats _memoryStats = dev::SystemInfo::MemoryStats();
    dev::SystemInfo::GPUStats _gpuStats = dev::SystemInfo::GPUStats();
    dev::FrameProfiler::FrameInfo _frameInfo = dev::FrameProfiler::FrameInfo();
    dev::FrameProfiler::FrameInfo _frameWorkloadInfo = dev::FrameProfiler::FrameInfo();

    void _showSystemInfo() const;

    void _showFrameInfo() const;

    void _showFrameResourcesInfo() const noexcept;

    void _updateStats();
};
} // namespace vax::editor