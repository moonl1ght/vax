#pragma once

#include "luna.h"
#include "vkEngine.h"
#include "window.h"
#undef Status
#include "imgui.h"
#include <optional>

namespace vax::ui {
class UIEngine final {
  public:
    UIEngine(vax::vk::Engine& vkEngine, vax::vk::Window& window)
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

    vax::vk::Engine& engine() { return _vkEngine.get(); }

    const vax::vk::Engine& engine() const { return _vkEngine.get(); }

  private:
    ImDrawData* _imguiDrawData = nullptr;
    VkDescriptorPool _imguiDescriptorPool = VK_NULL_HANDLE;

    std::reference_wrapper<vax::vk::Window> _window;
    std::reference_wrapper<vax::vk::Engine> _vkEngine;
};
} // namespace vax::ui