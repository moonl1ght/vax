#pragma once

#include "baseRenderer.h"
#include "commandBuffer.h"
#include "drawableScene.h"
#include "frameProfiler.h"
#include "frameTime.h"
#include "renderPassDescriptor.h"
#include "renderPassGraph.h"
#include "renderPassGraphManager.h"
#include "uiEngine.h"
#include "vkEngine.h"

namespace vax::engine {
class Renderer final : public BaseRenderer {
  public:
    enum class RendererMode {
        ROVER_DEMO_WITH_ROVER_CAMERA,
        ROVER_DEMO_WITHOUT_ROVER_CAMERA,
    };

    Renderer(vax::vk::Engine& vkEngine, vax::ui::UIEngine& uiEngine)
        : BaseRenderer(vkEngine, uiEngine) {};

    ~Renderer() {};

    Renderer(const Renderer& other) = delete;
    Renderer& operator=(const Renderer& other) = delete;
    Renderer(Renderer&& other) noexcept = delete;
    Renderer& operator=(Renderer&& other) noexcept = delete;

    bool render(DrawableScene* scene, const FrameTime& frameTime);

    void setup();

    void prepare(DrawableScene* scene);

    void linkFrameProfiler(vax::FrameProfiler* frameProfiler) noexcept {
        _frameProfiler = frameProfiler;
    }

    void unlinkFrameProfiler(vax::FrameProfiler* frameProfiler) noexcept {
        if (_frameProfiler == frameProfiler) {
            _frameProfiler = nullptr;
        }
    }

  private:
    struct RenderPassInfo {
        vax::vk::CommandBuffer* commandBuffer;
        vax::vk::RenderPassDescriptor* renderPassDescriptor;
        VkPipelineLayout pipelineLayout;
        vax::engine::DrawableScene* scene;
        uint32_t imageIndex;
    };

    vax::Logger _logger = vax::Logger("Renderer");

    FrameProfiler* _frameProfiler = nullptr;

    std::unique_ptr<vax::engine::RenderPassGraphManager> _roverDemoPassGraphManager;

    std::unique_ptr<vax::engine::RenderPassGraph> _renderPassGraph;

    std::unique_ptr<vax::engine::RenderPassGraph> _uiPassGraph;

    RendererMode _rendererMode = RendererMode::ROVER_DEMO_WITHOUT_ROVER_CAMERA;

    bool _wasResized = false;

    bool _updateCommandBuffer(
        vax::vk::CommandBuffer& commandBuffer,
        uint32_t imageIndex,
        uint32_t roverCameraImageIndex,
        vax::engine::DrawableScene* scene
    );

    bool _drawScene(vax::vk::CommandBuffer& commandBuffer, vax::engine::DrawableScene* scene);

    void _drawUi(vax::vk::CommandBuffer& commandBuffer);

    bool _bindGlobalDescriptorSet(vax::vk::CommandBuffer& commandBuffer, VkPipelineLayout pipelineLayout);

    void _resize();

    void _writeFinalBlendDescriptorSets();

    void _writeRoverCameraDescriptorSets();

    void _rebuildRenderPassGraph(bool withRoverCamera);
};
} // namespace vax::engine