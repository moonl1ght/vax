#include "windowController.h"

using namespace vax::editor;

void WindowController::setupWindow(uint32_t index, math::SizeUI size, const std::string& name) {
    if (index >= maxWindows) {
        _logger.error("Invalid window index");
        return;
    }
    if (_windows[index] != nullptr) {
        _logger.error("Window already setup");
        return;
    }
    _windows[index] = std::make_unique<rhi::Window>(size.width, size.height);
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
    _windows[index]->destroy();
    _windows[index] = nullptr;
}

void WindowController::destroyAllWindows() {
    for (uint32_t i = 0; i < maxWindows; ++i) {
        if (_windows[i] != nullptr) {
            destroyWindow(i);
        }
    }
}