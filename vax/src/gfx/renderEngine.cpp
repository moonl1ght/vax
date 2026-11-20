#include "renderEngine.h"

using namespace vax;
using namespace vax::gfx;

bool RenderEngine::setup(WindowController& windowController) noexcept {
    _vkEngine = std::make_unique<rhi::Engine>(windowController);
    if (!_vkEngine->setup()) {
        return false;
    }
    _uiEngine = std::make_unique<ui::UIEngine>(*_vkEngine, *windowController.getWindow(0));

    auto& descriptorRegistry = _vkEngine->pipelineManager->descriptorRegistry();
    _resourceManager = std::make_unique<vax::rhi::ResourceManager>(*_vkEngine->device, descriptorRegistry);
    if (!_resourceManager->setup()) {
        return false;
    }

    _renderer = std::make_unique<gfx::Renderer>(*_vkEngine, *_uiEngine);
    _renderer->setup(*_resourceManager);
    _logger.success("Rendering engine setup complete!");
    return true;
}

void RenderEngine::waitIdle() noexcept { _vkEngine->device->waitIdle(); }

void RenderEngine::cleanup() noexcept {
    _logger.info("Starting rendering engine cleanup...");
    _uiEngine->cleanup();
    _renderer = nullptr;
    _resourceManager = nullptr;
    _uiEngine = nullptr;
    _vkEngine->cleanup();
    _vkEngine = nullptr;
    _logger.success("Rendering engine cleanup complete!");
}