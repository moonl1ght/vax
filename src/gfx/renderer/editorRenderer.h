#pragma once

#include "baseRenderer.h"
#include "editorScene.h"
#include "frameTime.h"
#include "renderPassGraph.h"
#include "windowController.h"

namespace vax::gfx {

class EditorRenderer final : public gfx::BaseRenderer {
  public:
    explicit EditorRenderer(
        rhi::VulkanEngine& vkEngine,
        rhi::ResourceManager& resourceManager,
        editor::WindowController& windowController,
        editor::UIEngine& uiEngine
    )
        : BaseRenderer(vkEngine, resourceManager)
        , _windowController(windowController)
        , _uiEngine(uiEngine) {};

    ~EditorRenderer() override = default;

    EditorRenderer(const EditorRenderer&) = delete;
    EditorRenderer& operator=(const EditorRenderer&) = delete;
    EditorRenderer(EditorRenderer&&) = delete;
    EditorRenderer& operator=(EditorRenderer&&) = delete;

    bool setup() noexcept;

    bool render(scene::EditorScene* scene, const time::FrameTime& frameTime);

  private:
    dev::Logger _logger = dev::Logger("EditorRenderer");

    std::reference_wrapper<editor::WindowController> _windowController;
    std::reference_wrapper<editor::UIEngine> _uiEngine;

    std::unique_ptr<RenderPassGraph> _uiRenderPassGraph;
    std::unique_ptr<RenderPassGraph> _sceneRenderPassGraph;

    bool _updateCommandBuffer(vax::rhi::CommandBuffer& commandBuffer, uint32_t imageIndex, scene::EditorScene* scene);

    void _resize(scene::EditorScene* scene) noexcept;

    bool _submit(vax::rhi::CommandBuffer& commandBuffer) noexcept;

    bool _present(scene::EditorScene* scene, uint32_t imageIndex) noexcept;

    void _drawUI(vax::rhi::CommandBuffer& commandBuffer, uint32_t imageIndex) noexcept;

    void _drawScene(vax::rhi::CommandBuffer& commandBuffer, scene::EditorScene* scene, uint32_t imageIndex) noexcept;

    bool _buildRenderPassGraph() noexcept;

    bool _buildRenderTarget() noexcept;

    bool _buildPipelines() noexcept;

    bool _prepareDrawRanges(scene::EditorScene* scene) noexcept;
};

} // namespace vax::gfx