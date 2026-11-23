#include "window.h"
#include <SDL3/SDL_vulkan.h>

using namespace vax::rhi;
using namespace vax;

bool vax::rhi::Window::load(bool visible, bool resizable) {
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);
    if (!visible) {
        window_flags |= SDL_WINDOW_HIDDEN;
    }
    if (resizable) {
        window_flags |= SDL_WINDOW_RESIZABLE;
    }
    window = SDL_CreateWindow(_name.c_str(), width, height, window_flags);
    if (window == nullptr) {
        _logger.error("Failed to create window");
        return false;
    }
    _visible = visible;
    return true;
}

bool vax::rhi::Window::createSurface(VkInstance instance) {
    _instance = instance;
    if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface)) {
        _logger.error("Failed to create surface");
        return false;
    }
    return true;
}

void vax::rhi::Window::destroySurface() {
    if (surface != VK_NULL_HANDLE && _instance != VK_NULL_HANDLE) {
        _logger.info("Destroying surface...");
        vkDestroySurfaceKHR(_instance, surface, nullptr);
        surface = VK_NULL_HANDLE;
        _instance = VK_NULL_HANDLE;
    }
}

void vax::rhi::Window::destroyWindow() {
    if (window != nullptr) {
        _logger.info("Destroying window...");
        SDL_DestroyWindow(window);
        window = nullptr;
    }
}

void vax::rhi::Window::destroy() {
    destroyWindow();
    destroySurface();
}

void vax::rhi::Window::show() {
    if (window != nullptr) {
        if (_windowWillShowCallback) {
            _windowWillShowCallback();
        }
        SDL_ShowWindow(window);
        _visible = true;
        if (_windowDidShowCallback) {
            _windowDidShowCallback();
        }
    }
}

void vax::rhi::Window::hide() {
    if (window != nullptr) {
        if (_windowWillHideCallback) {
            _windowWillHideCallback();
        }
        SDL_HideWindow(window);
        _visible = false;
        if (_windowDidHideCallback) {
            _windowDidHideCallback();
        }
    }
}

void vax::rhi::Window::bringToFront() {
    if (window != nullptr) {
        SDL_RaiseWindow(window);
    }
}

void vax::rhi::Window::setWindowName(const std::string& name) {
    _name = name;
    if (window != nullptr) {
        SDL_SetWindowTitle(window, _name.c_str());
    }
}

void vax::rhi::Window::setWindowWillHideCallback(std::function<void()> callback) { _windowWillHideCallback = callback; }

void vax::rhi::Window::setWindowWillShowCallback(std::function<void()> callback) { _windowWillShowCallback = callback; }

void vax::rhi::Window::setWindowDidHideCallback(std::function<void()> callback) { _windowDidHideCallback = callback; }

void vax::rhi::Window::setWindowDidShowCallback(std::function<void()> callback) { _windowDidShowCallback = callback; }

bool vax::rhi::Window::createSwapchain(const vax::rhi::Device& device, VkPresentModeKHR presentMode) {
    _swapchain = std::make_unique<vax::rhi::Swapchain>(device, surface, vax::math::SizeUI{width, height});
    if (!_swapchain->setup(presentMode)) {
        return false;
    }
    return true;
}

void vax::rhi::Window::cleanupSwapchain() {
    if (_swapchain != nullptr) {
        _swapchain->cleanup();
        _swapchain = nullptr;
    }
}
