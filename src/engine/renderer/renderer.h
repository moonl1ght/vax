#pragma once

#include "baseRenderer.h"
#include "commandBuffer.h"
#include "drawableScene.h"
#include "frameProfiler.h"
#include "frameTime.h"
#include "renderPassDescriptor.h"
#include "renderPassGraphDescriptor.h"
#include "renderPassGraphManager.h"
#include "state_tracker.h"
#include "uiEngine.h"
#include "vkEngine.h"

namespace vax::engine {
class Renderer final : public BaseRenderer {
  public:
    state_tracker<RenderPassGraphMode> renderPassGraphMode =
        state_tracker<RenderPassGraphMode>(RenderPassGraphMode::UNKNOWN);

    Renderer(vax::vk::Engine& vkEngine, vax::ui::UIEngine& uiEngine)
        : BaseRenderer(vkEngine, uiEngine) {
        _renderPassGraphManager = std::make_unique<RenderPassGraphManager>(
            *_vkEngine.get().device,
            *_vkEngine.get().pipelineManager,
            *_vkEngine.get().descriptorSetManager,
            _vkEngine.get().getWindowController(),
            _uiEngine.get()
        );
    };

    ~Renderer() {};

    Renderer(const Renderer& other) = delete;
    Renderer& operator=(const Renderer& other) = delete;
    Renderer(Renderer&& other) noexcept = delete;
    Renderer& operator=(Renderer&& other) noexcept = delete;

    void prepareForRender(DrawableScene* scene);

    bool render(DrawableScene* scene, const FrameTime& frameTime);

    void load();

    void setup(DrawableScene* scene);

    void linkFrameProfiler(vax::FrameProfiler* frameProfiler) noexcept { _frameProfiler = frameProfiler; }

    void unlinkFrameProfiler(vax::FrameProfiler* frameProfiler) noexcept {
        if (_frameProfiler == frameProfiler) {
            _frameProfiler = nullptr;
        }
    }

    RenderPassGraphManager* renderPassGraphManager() { return _renderPassGraphManager.get(); }

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

    std::unique_ptr<vax::engine::RenderPassGraphManager> _renderPassGraphManager;

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

    void _writeSecondaryWindowDescriptorSets();
};
} // namespace vax::engine