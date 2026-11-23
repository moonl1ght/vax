#include "editor.h"
#include "renderEngine.h"
#include "viewBuilder.h"

using namespace vax::editor;

bool Editor::setup() noexcept {
    _windowController = std::make_unique<WindowController>();
    _windowController->setupWindow(0, vax::math::SizeUI{1920, 1080}, "VAX");
    if (!_windowController->getWindow(0)->setup(true, true)) {
        _logger.failure("Failed to load window!");
        return false;
    }
    return true;
}

void Editor::createUIEngine(rhi::VulkanEngine& vkEngine) noexcept {
    _uiEngine = std::make_unique<UIEngine>(vkEngine, *_windowController->getWindow(0));
}

void Editor::loadMainView(gfx::RenderEngine& renderEngine) noexcept {
    _viewBuilder = std::make_unique<ViewBuilder>(
        *_windowController, _inputController, renderEngine.editorRenderer(), renderEngine
    );
    _viewManager = std::make_unique<ViewManager>(*_uiEngine, *_viewBuilder);
    _viewManager->setRootView(_viewBuilder->buildStartingView());
}

void Editor::cleanup() noexcept {
    _viewManager = nullptr;
    _viewBuilder = nullptr;
    if (_uiEngine) {
        _uiEngine->cleanup();
        _uiEngine = nullptr;
    }
    _windowController->destroyAllWindows();
    _windowController = nullptr;
}