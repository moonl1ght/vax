#include "statsView.h"
#include "IconsFontAwesome6.h"
#include "closeButton.h"
#include "debouncer.h"
#include <imgui.h>
#include <ratio>

using namespace vax::editor;

void StatsView::updateUI(const vax::time::FrameTime& frameTime) {
    _fullStatsView->updateUI(frameTime);

    if (!_isShown) {
        return;
    }
    const ImGuiViewport* mainViewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowViewport(mainViewport->ID);
    ImGui::SetNextWindowPos(
        ImVec2(mainViewport->WorkPos.x + mainViewport->WorkSize.x - 10.0f, mainViewport->WorkPos.y + 10.0f),
        ImGuiCond_Always,
        ImVec2(1.0f, 0.0f)
    );
    ImGui::SetNextWindowSize(SIZE, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.35f);

    constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                                       ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar |
                                       ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking |
                                       ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.33f, 0.55f, 0.95f, 1.00f));
    const bool isOpen = ImGui::Begin("Stats", &_isShown, flags);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
    if (!isOpen) {
        ImGui::End();
        return;
    }

    CloseButton::topCorner(_isShown);

    if (ImGui::SmallButton(ICON_FA_CHART_LINE " Full stats")) {
        _fullStatsView->setShown(!_fullStatsView->isShown());
    }
    ImGui::Spacing();

    _updateStats();
    _showSystemInfo();
    // ImGui::Spacing();
    // ImGui::Separator();
    // ImGui::Spacing();
    // _showFrameInfo();
    // ImGui::Spacing();
    // ImGui::Separator();
    // ImGui::Spacing();
    _showFrameResourcesInfo();
    ImGui::End();
}

void StatsView::_showSystemInfo() const {
    ImGui::Text("RAM usage: %.2f%% (%dMB / %dMB)", _memoryStats.usagePercentage, _memoryStats.used, _memoryStats.total);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("GPU Info:");
    ImGui::Text("%s", _gpuStats.name.c_str());
    ImGui::Text(
        "Usage: %.2f%% (%dMB / %dMB)",
        _gpuStats.memoryStats.usagePercentage,
        _gpuStats.memoryStats.used,
        _gpuStats.memoryStats.total
    );
}

void StatsView::_showFrameInfo() const {
    // if (_frameProfiler) {
    //     using float_milliseconds = std::chrono::duration<float, std::milli>;
    //     auto duration = float_milliseconds(_frameInfo.duration).count();
    //     ImGui::Text("Frame Info:");
    //     ImGui::Text("VSYNC FB: %.2fms, FPS: %d", duration, _frameInfo.fps);

    //     auto workloadDuration = float_milliseconds(_frameWorkloadInfo.duration).count();
    //     if (duration > 0.0f) {
    //         ImGui::Text(
    //             "Used FT: %.2fms / %.2fms (%.2f%%)", workloadDuration, duration, (workloadDuration * 100.0f) /
    //             duration
    //         );
    //     } else {
    //         ImGui::Text("Used FT: %.2fms / %.2fms", workloadDuration, duration);
    //     }
    // }
}

void StatsView::_showFrameResourcesInfo() const noexcept {
    // auto& descriptorRegistry = _renderEngine.get().vkEngine().pipelineManager->descriptorRegistry();

    // ImGui::Text("Pipeline state:");

    // ImGui::Text("Descriptor registry state:");
    // ImGui::Text("Storage textures: %d", descriptorRegistry.stats().storageTextures);
    // ImGui::Text("Sampled textures: %d", descriptorRegistry.stats().sampledTextures);
    // ImGui::Text("Storage buffers: %d", descriptorRegistry.stats().storageBuffers);
    // ImGui::Text("Samplers: %d", descriptorRegistry.stats().samplers);
}

void StatsView::_updateStats() {
    _debouncer.execute([this]() {
        _memoryStats = _systemInfo.getRAMStats();
        _gpuStats = _systemInfo.getGPUStats();
        if (_frameProfiler) {
            auto frameInfo = _frameProfiler->getFrameZoneInfo("frame");
            if (frameInfo) {
                _frameInfo = *frameInfo;
            }
            auto frameWorkloadInfo = _frameProfiler->getFrameZoneInfo("frame_workload");
            if (frameWorkloadInfo) {
                _frameWorkloadInfo = *frameWorkloadInfo;
            }
        }
    });
}
