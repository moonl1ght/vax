#pragma once

#include "vulkanCommon.h"
#include <SDL3/SDL.h>
#include "vkEngine.h"
#include "window.h"
#undef Status
#include "imgui.h"
#include <optional>

namespace vax::ui {
class UIEngine final {
  public:
    UIEngine(vax::rhi::Engine& vkEngine, vax::rhi::Window& window)
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

    vax::rhi::Engine& engine() { return _vkEngine.get(); }

    const vax::rhi::Engine& engine() const { return _vkEngine.get(); }

  private:
    ImDrawData* _imguiDrawData = nullptr;
    VkDescriptorPool _imguiDescriptorPool = VK_NULL_HANDLE;

    std::reference_wrapper<vax::rhi::Window> _window;
    std::reference_wrapper<vax::rhi::Engine> _vkEngine;
};
} // namespace vax::ui