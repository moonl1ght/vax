#include "window.h"
#include <SDL3/SDL_vulkan.h>

using namespace vax::vk;
using namespace vax;

bool vax::vk::Window::load(bool visible, bool resizable) {
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

bool vax::vk::Window::createSurface(VkInstance instance) {
    _instance = instance;
    if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface)) {
        _logger.error("Failed to create surface");
        return false;
    }
    return true;
}

void vax::vk::Window::destroySurface() {
    if (surface != VK_NULL_HANDLE && _instance != VK_NULL_HANDLE) {
        _logger.info("Destroying surface...");
        vkDestroySurfaceKHR(_instance, surface, nullptr);
        surface = VK_NULL_HANDLE;
        _instance = VK_NULL_HANDLE;
    }
}

void vax::vk::Window::destroyWindow() {
    if (window != nullptr) {
        _logger.info("Destroying window...");
        SDL_DestroyWindow(window);
        window = nullptr;
    }
}

void vax::vk::Window::destroy() {
    destroyWindow();
    destroySurface();
}

void vax::vk::Window::show() {
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

void vax::vk::Window::hide() {
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

void vax::vk::Window::bringToFront() {
    if (window != nullptr) {
        SDL_RaiseWindow(window);
    }
}

void vax::vk::Window::setWindowName(const std::string& name) {
    _name = name;
    if (window != nullptr) {
        SDL_SetWindowTitle(window, _name.c_str());
    }
}

void vax::vk::Window::setWindowWillHideCallback(std::function<void()> callback) { _windowWillHideCallback = callback; }

void vax::vk::Window::setWindowWillShowCallback(std::function<void()> callback) { _windowWillShowCallback = callback; }

void vax::vk::Window::setWindowDidHideCallback(std::function<void()> callback) { _windowDidHideCallback = callback; }

void vax::vk::Window::setWindowDidShowCallback(std::function<void()> callback) { _windowDidShowCallback = callback; }

void vax::vk::Window::createSwapchain(const vax::vk::Device& device, VkPresentModeKHR presentMode) {
    _swapchain = std::make_unique<vax::vk::Swapchain>(device, surface, vax::math::SizeUI{width, height});
    if (!_swapchain->setup(presentMode)) {
        _logger.error("Failed to create swapchain!");
        return;
    }
}

void vax::vk::Window::cleanupSwapchain() {
    if (_swapchain != nullptr) {
        _swapchain->cleanup();
        _swapchain = nullptr;
    }
}
