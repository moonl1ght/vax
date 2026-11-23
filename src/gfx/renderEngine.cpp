#include "renderEngine.h"

using namespace vax::gfx;

bool RenderEngine::setup(editor::Editor* editor) noexcept {
    _vkEngine = std::make_unique<rhi::VulkanEngine>();
    if (!_vkEngine->createInstance()) {
        return false;
    }
    if (editor) {
        auto mainWindow = editor->windowController().getWindow(0);
        auto vkWindow = std::unique_ptr<rhi::VulkanWindow>();
        vkWindow->setup(_vkEngine->instance, mainWindow->sdlWindow());
        if (!_vkEngine->setup(vkWindow->surface())) {
            return false;
        }
        vkWindow->createSwapchain(*_vkEngine->device, mainWindow->width, mainWindow->height, VK_PRESENT_MODE_FIFO_KHR);
        mainWindow->attachVkWindow(std::move(vkWindow));
    } else {
        if (!_vkEngine->setup(VK_NULL_HANDLE)) {
            return false;
        }
    }

    auto& descriptorRegistry = _vkEngine->pipelineManager->descriptorRegistry();
    _resourceManager = std::make_unique<vax::rhi::ResourceManager>(*_vkEngine->device, descriptorRegistry);
    if (!_resourceManager->setup()) {
        return false;
    }

    _renderer = std::make_unique<gfx::Renderer>(*_vkEngine, editor->uiEngine(), editor->windowController());
    _renderer->setup(*_resourceManager);
    _logger.success("Rendering engine setup complete!");
    return true;
}

void RenderEngine::waitIdle() noexcept { _vkEngine->device->waitIdle(); }

void RenderEngine::cleanup() noexcept {
    _logger.info("Starting rendering engine cleanup...");
    _renderer = nullptr;
    _resourceManager = nullptr;
    _vkEngine->cleanup();
    _vkEngine = nullptr;
    _logger.success("Rendering engine cleanup complete!");
}