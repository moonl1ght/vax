#pragma once

#include "logger.h"
#include "vkWindow.h"
#include <SDL3/SDL.h>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace vax::editor {
class Window final {
  public:
    uint32_t width = 1920;
    uint32_t height = 1080;

    explicit Window(uint32_t width, uint32_t height) noexcept
        : width(width)
        , height(height) {};

    Window(const Window& other) = delete;
    Window(Window&& other) = delete;
    Window& operator=(const Window& other) = delete;
    Window& operator=(Window&& other) = delete;

    bool setup(bool visible, bool resizable) noexcept;

    void attachVkWindow(std::unique_ptr<rhi::VulkanWindow> vkWindow) noexcept { _vkWindow = std::move(vkWindow); }

    void destroy();

    void show();

    void bringToFront();

    void hide();

    void setWindowWillHideCallback(std::function<void()> callback);

    void setWindowWillShowCallback(std::function<void()> callback);

    void setWindowDidHideCallback(std::function<void()> callback);

    void setWindowDidShowCallback(std::function<void()> callback);

    SDL_WindowID getWindowID() const { return SDL_GetWindowID(_window); }

    void setWindowName(const std::string& name);

    SDL_Window* sdlWindow() const { return _window; }

    rhi::VulkanWindow* vkWindow() const { return _vkWindow.get(); }

  private:
    dev::Logger _logger = dev::Logger("Window");

    std::string _name = "VAX";
    SDL_Window* _window = nullptr;

    std::unique_ptr<rhi::VulkanWindow> _vkWindow;

    std::function<void()> _windowWillHideCallback;
    std::function<void()> _windowWillShowCallback;
    std::function<void()> _windowDidHideCallback;
    std::function<void()> _windowDidShowCallback;

    bool _visible = false;
};
}; // namespace vax::editor
