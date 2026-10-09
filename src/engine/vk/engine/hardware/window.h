#pragma once

#include "luna.h"
#include "device.h"
#include "swapchain.h"
#include <functional>

namespace vax::vk {
class Window final {
  public:
    uint32_t width = 1920;
    uint32_t height = 1080;
    SDL_Window* window = nullptr;
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    explicit Window(uint32_t width, uint32_t height) noexcept
        : width(width)
        , height(height) {};

    Window(const Window& other) = delete;
    Window(Window&& other) = delete;
    Window& operator=(const Window& other) = delete;
    Window& operator=(Window&& other) = delete;

    bool load(bool visible, bool resizable);
    bool createSurface(VkInstance instance);
    void destroySurface();
    void destroyWindow();
    void destroy();

    void createSwapchain(const vax::vk::Device& device, VkPresentModeKHR presentMode);

    void cleanupSwapchain();

    void show();
    void bringToFront();
    void hide();

    void setWindowWillHideCallback(std::function<void()> callback);

    void setWindowWillShowCallback(std::function<void()> callback);

    void setWindowDidHideCallback(std::function<void()> callback);

    void setWindowDidShowCallback(std::function<void()> callback);

    SDL_WindowID getWindowID() const { return SDL_GetWindowID(window); }

    void setWindowName(const std::string& name);

    const vax::vk::Swapchain* getSwapchain() const { return _swapchain.get(); }

    vax::vk::Swapchain* getSwapchain() { return _swapchain.get(); }

  private:
    VkInstance _instance = VK_NULL_HANDLE;
    vax::Logger _logger = vax::Logger("Window");
    std::string _name = "VAX";
    std::unique_ptr<vax::vk::Swapchain> _swapchain;
    std::function<void()> _windowWillHideCallback;
    std::function<void()> _windowWillShowCallback;
    std::function<void()> _windowDidHideCallback;
    std::function<void()> _windowDidShowCallback;
    bool _visible = false;
};
}; // namespace vax::vk
