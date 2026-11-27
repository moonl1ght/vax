#pragma once

#include "baseRenderer.h"
#include "commandBuffer.h"
#include "frameProfiler.h"
#include "frameTime.h"
#include "renderPassDescriptor.h"
#include "renderPassGraphDescriptor.h"
#include "renderPassGraphManager.h"
#include "resourceManager.h"
#include "scene.h"
#include "shaderResourceHandle.h"
#include "state_tracker.h"
#include "uiEngine.h"
#include "vkEngine.h"
#include <optional>

namespace vax::gfx {
class EditorRenderer_old final : public BaseRenderer {
  public:
    state_tracker<RenderPassGraphMode> renderPassGraphMode =
        state_tracker<RenderPassGraphMode>(RenderPassGraphMode::UNKNOWN);

    EditorRenderer_old(
        rhi::VulkanEngine& vkEngine,
        rhi::ResourceManager& resourceManager,
        editor::UIEngine& uiEngine,
        editor::WindowController& windowController
    )
        : BaseRenderer(vkEngine, resourceManager)
        , _uiEngine(uiEngine)
        , _windowController(windowController) {};

    ~EditorRenderer_old() {};

    EditorRenderer_old(const EditorRenderer_old& other) = delete;
    EditorRenderer_old& operator=(const EditorRenderer_old& other) = delete;
    EditorRenderer_old(EditorRenderer_old&& other) noexcept = delete;
    EditorRenderer_old& operator=(EditorRenderer_old&& other) noexcept = delete;

    bool render(Scene* scene, const time::FrameTime& frameTime);

    void setup(vax::rhi::ResourceManager& resourceManager) noexcept;

    void load();

    bool isLoaded() const noexcept { return _isLoaded; }

    RenderPassGraphManager* renderPassGraphManager() { return _renderPassGraphManager.get(); }

  private:
    struct SwapchainResult {
        uint32_t imageIndex;
        bool shouldRecreateSwapchain;
        bool hasError;
    };

    struct RenderPassInfo {
        vax::rhi::CommandBuffer* commandBuffer;
        vax::rhi::RenderPassDescriptor* renderPassDescriptor;
        VkPipelineLayout pipelineLayout;
        vax::gfx::Scene* scene;
        uint32_t imageIndex;
    };

    dev::Logger _logger = dev::Logger("EditorRenderer_old");

    std::reference_wrapper<editor::WindowController> _windowController;

    std::reference_wrapper<editor::UIEngine> _uiEngine;

    std::unique_ptr<vax::gfx::RenderPassGraphManager> _renderPassGraphManager;

    bool _wasResized = false;

    bool _isLoaded = false;

    bool _updateCommandBuffer(
        vax::rhi::CommandBuffer& commandBuffer,
        uint32_t imageIndex,
        uint32_t agentCameraImageIndex,
        vax::gfx::Scene* scene
    );

    bool _drawScene(vax::rhi::CommandBuffer& commandBuffer, vax::gfx::Scene* scene);

    void _drawUi(vax::rhi::CommandBuffer& commandBuffer);

    bool _bindDescriptorSets(vax::rhi::CommandBuffer& commandBuffer);

    void _resize();

    void _resizeSecondary();

    void _prepareForRender(Scene* scene);

    rhi::Swapchain* _getSwapchain(size_t index);
};
} // namespace vax::gfx