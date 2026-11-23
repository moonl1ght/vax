#include "editor.h"

using namespace vax::editor;

bool Editor::setup() noexcept {
    _windowController = std::make_unique<WindowController>();
    _windowController->setupWindow(0, vax::math::SizeUI{1920, 1080}, "VAX");
    if (!_windowController->getWindow(0)->load(true, true)) {
        _logger.failure("Failed to load window!");
        return false;
    }

    // _viewBuilder = std::make_unique<ViewBuilder>(*_windowController, _inputController, *_renderEngine);
    // _viewManager = std::make_unique<ViewManager>(_renderEngine->uiEngine(), *_viewBuilder);
    // _viewManager->setRootView(_viewBuilder->buildStartingView());
    return true;
}

void Editor::cleanup() noexcept {
    _viewManager = nullptr;
    _viewBuilder = nullptr;
    _uiEngine = nullptr;
    _windowController->destroyAllWindows();
    _windowController = nullptr;
}