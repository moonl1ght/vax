#include "window.h"
#include <SDL3/SDL_vulkan.h>

using namespace vax::rhi;
using namespace vax;

bool vax::rhi::Window::setup(bool visible, bool resizable) noexcept {
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);
    if (!visible) {
        window_flags |= SDL_WINDOW_HIDDEN;
    }
    if (resizable) {
        window_flags |= SDL_WINDOW_RESIZABLE;
    }
    _window = SDL_CreateWindow(_name.c_str(), width, height, window_flags);
    if (_window == nullptr) {
        _logger.error("Failed to create window");
        return false;
    }
    _visible = visible;
    return true;
}

void vax::rhi::Window::destroy() {
    if (_window != nullptr) {
        _logger.info("Destroying window...");
        SDL_DestroyWindow(_window);
        _window = nullptr;
        if (_vkWindow) {
            _vkWindow->cleanup();
            _vkWindow = nullptr;
        }
    }
}

void vax::rhi::Window::show() {
    if (_window != nullptr) {
        if (_windowWillShowCallback) {
            _windowWillShowCallback();
        }
        SDL_ShowWindow(_window);
        _visible = true;
        if (_windowDidShowCallback) {
            _windowDidShowCallback();
        }
    }
}

void vax::rhi::Window::hide() {
    if (_window != nullptr) {
        if (_windowWillHideCallback) {
            _windowWillHideCallback();
        }
        SDL_HideWindow(_window);
        _visible = false;
        if (_windowDidHideCallback) {
            _windowDidHideCallback();
        }
    }
}

void vax::rhi::Window::bringToFront() {
    if (_window != nullptr) {
        SDL_RaiseWindow(_window);
    }
}

void vax::rhi::Window::setWindowName(const std::string& name) {
    _name = name;
    if (_window != nullptr) {
        SDL_SetWindowTitle(_window, _name.c_str());
    }
}

void vax::rhi::Window::setWindowWillHideCallback(std::function<void()> callback) { _windowWillHideCallback = callback; }

void vax::rhi::Window::setWindowWillShowCallback(std::function<void()> callback) { _windowWillShowCallback = callback; }

void vax::rhi::Window::setWindowDidHideCallback(std::function<void()> callback) { _windowDidHideCallback = callback; }

void vax::rhi::Window::setWindowDidShowCallback(std::function<void()> callback) { _windowDidShowCallback = callback; }
