#include "renderEngine.h"

using namespace vax::gfx;

bool RenderEngine::setup(editor::Editor* editor) noexcept {
    _vkEngine = std::make_unique<rhi::VulkanEngine>();
    if (!_vkEngine->createInstance()) {
        return false;
    }
    if (editor) {
        auto mainWindow = editor->windowController().getWindow(0);
        auto vkWindow = std::make_unique<rhi::VulkanWindow>();
        if (!vkWindow->setup(_vkEngine->instance, mainWindow->sdlWindow())) {
            return false;
        }
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

    if (!editor) {
        _logger.failure("Headless rendering is not supported yet!");
        return false;
    }
    editor->createUIEngine(*_vkEngine);
    _editorRenderer_old =
        std::make_unique<gfx::EditorRenderer_old>(*_vkEngine, editor->uiEngine(), editor->windowController());
    _editorRenderer_old->setup(*_resourceManager);
    _editorRenderer = std::make_unique<gfx::EditorRenderer>(*_vkEngine, editor->windowController(), editor->uiEngine());
    _editorRenderer->setup(*_resourceManager);
    _logger.success("Rendering engine setup complete!");
    return true;
}

void RenderEngine::waitIdle() noexcept { _vkEngine->device->waitIdle(); }

void RenderEngine::cleanup() noexcept {
    _logger.info("Starting rendering engine cleanup...");
    _editorRenderer_old = nullptr;
    _editorRenderer = nullptr;
    _simulationRenderer = nullptr;
    _resourceManager = nullptr;
    _vkEngine->cleanup();
    _vkEngine = nullptr;
    _logger.success("Rendering engine cleanup complete!");
}