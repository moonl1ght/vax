#include "statsView.h"
#include "debouncer.h"
#include <imgui.h>
#include <ratio>

using namespace vax::ui;

void StatsView::updateUI(const vax::engine::FrameTime& frameTime) {
    if (!_isShown) {
        return;
    }
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImVec2 top_right = ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - 10.0f, viewport->WorkPos.y + 10.0f);

    ImGui::SetNextWindowPos(top_right, ImGuiCond_FirstUseEver, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(480, 250), ImGuiCond_FirstUseEver);
    ImGuiWindowClass windowClass;
    windowClass.ViewportFlagsOverrideSet = ImGuiViewportFlags_NoAutoMerge;
    ImGui::SetNextWindowClass(&windowClass);
    if (!ImGui::Begin("Stats", &_isShown, ImGuiWindowFlags_NoDocking)) {
        ImGui::End();
        return;
    }

    _updateStats();
    _showSystemInfo();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    _showFrameInfo();
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
    if (_frameProfiler) {
        using float_milliseconds = std::chrono::duration<float, std::milli>;
        auto duration = float_milliseconds(_frameInfo.duration).count();
        ImGui::Text("Frame Info:");
        ImGui::Text("VSYNC FB: %.2fms, FPS: %d", duration, _frameInfo.fps);

        auto workloadDuration = float_milliseconds(_frameWorkloadInfo.duration).count();
        if (duration > 0.0f) {
            ImGui::Text(
                "Used FT: %.2fms / %.2fms (%.2f%%)", workloadDuration, duration, (workloadDuration * 100.0f) / duration
            );
        } else {
            ImGui::Text("Used FT: %.2fms / %.2fms", workloadDuration, duration);
        }
    }
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