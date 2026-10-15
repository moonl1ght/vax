#include "windowController.h"

using namespace vax;
using namespace vax::vk;

void WindowController::setupWindow(uint32_t index, vax::math::SizeUI size, const std::string& name) {
    if (index >= maxWindows) {
        _logger.error("Invalid window index");
        return;
    }
    if (_windows[index] != nullptr) {
        _logger.error("Window already setup");
        return;
    }
    _windows[index] = std::make_unique<vax::vk::Window>(size.width, size.height);
    _windows[index]->setWindowName(name);
}

void WindowController::destroyWindow(uint32_t index) {
    if (index >= maxWindows) {
        _logger.error("Invalid window index");
        return;
    }
    if (_windows[index] == nullptr) {
        _logger.error("Window not setup");
        return;
    }
    _windows[index]->destroyWindow();
    _windows[index] = nullptr;
}

void WindowController::destroyAllWindows() {
    for (uint32_t i = 0; i < maxWindows; ++i) {
        destroyWindow(i);
    }
}