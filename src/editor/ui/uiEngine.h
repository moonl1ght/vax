#pragma once

#include "vkEngine.h"
#include "vulkanCommon.h"
#include "window.h"
#include <SDL3/SDL.h>
#undef Status
#include "imgui.h"
#include <optional>

namespace vax::editor {
class UIEngine final {
  public:
    UIEngine(vax::rhi::VulkanEngine& vkEngine, vax::rhi::Window& window)
        : _vkEngine(vkEngine)
        , _window(window) {}

    ~UIEngine() {}

    UIEngine(const UIEngine& other) = delete;
    UIEngine& operator=(const UIEngine& other) = delete;
    UIEngine(UIEngine&& other) noexcept = delete;
    UIEngine& operator=(UIEngine&& other) noexcept = delete;

    void updateUiStart();

    void updateUiEnd();

    void render(VkCommandBuffer commandBuffer);
    void setup(VkRenderPass renderPass);
    void cleanup();
    void processEvents(SDL_Event& event);

    bool isSecondaryViewportEvent(const SDL_Event& event) const;

    vax::rhi::VulkanEngine& engine() { return _vkEngine.get(); }

    const vax::rhi::VulkanEngine& engine() const { return _vkEngine.get(); }

    ImFont* monoFont() const { return _monoFont; }

  private:
    static constexpr float UI_FONT_SIZE = 17.0f;
    static constexpr float MONO_FONT_SIZE = 16.0f;

    void _setupFonts();
    void _applyTheme();

    ImFont* _monoFont = nullptr;
    ImDrawData* _imguiDrawData = nullptr;
    VkDescriptorPool _imguiDescriptorPool = VK_NULL_HANDLE;

    std::reference_wrapper<vax::rhi::Window> _window;
    std::reference_wrapper<vax::rhi::VulkanEngine> _vkEngine;
};
} // namespace vax::editor