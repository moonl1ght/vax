#include "window.h"
#include <SDL3/SDL_vulkan.h>

using namespace vax::editor;
using namespace vax;

bool Window::setup(bool visible, bool resizable) noexcept {
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

void Window::destroy() {
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

void Window::show() {
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

void Window::hide() {
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

void Window::bringToFront() {
    if (_window != nullptr) {
        SDL_RaiseWindow(_window);
    }
}

void Window::setWindowName(const std::string& name) {
    _name = name;
    if (_window != nullptr) {
        SDL_SetWindowTitle(_window, _name.c_str());
    }
}

void Window::setWindowWillHideCallback(std::function<void()> callback) { _windowWillHideCallback = callback; }

void Window::setWindowWillShowCallback(std::function<void()> callback) { _windowWillShowCallback = callback; }

void Window::setWindowDidHideCallback(std::function<void()> callback) { _windowDidHideCallback = callback; }

void Window::setWindowDidShowCallback(std::function<void()> callback) { _windowDidShowCallback = callback; }
