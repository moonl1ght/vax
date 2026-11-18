#include "renderingEngine.h"

using namespace vax::rendering;

bool RenderingEngine::setup(vax::WindowController& windowController) noexcept {
    _vkEngine = std::make_unique<vax::vk::Engine>(windowController);
    if (!_vkEngine->setup()) {
        return false;
    }
    _uiEngine = std::make_unique<ui::UIEngine>(*_vkEngine, *windowController.getWindow(0));
    _renderer = std::make_unique<engine::Renderer>(*_vkEngine, *_uiEngine);
    _renderer->setup(*_resourceManager);
    auto& descriptorRegistry = _vkEngine->pipelineManager->descriptorRegistry();
    auto resourceManager = std::make_unique<vax::vk::ResourceManager>(*_vkEngine->device, descriptorRegistry);
    resourceManager->setup();
    _logger.success("Rendering engine setup complete!");
    return true;
}

void RenderingEngine::waitIdle() noexcept { _vkEngine->device->waitIdle(); }

void RenderingEngine::cleanup() noexcept {
    _logger.info("Starting rendering engine cleanup...");
    _uiEngine->cleanup();
    _renderer = nullptr;
    _uiEngine = nullptr;
    _vkEngine = nullptr;
    _logger.success("Rendering engine cleanup complete!");
}