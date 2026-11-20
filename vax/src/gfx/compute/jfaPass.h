#pragma once

#include "pipelineManager.h"
#include "renderDestination.h"
#include "renderPassNode.h"
#include "texture.h"

namespace vax::gfx {
class JFAPass final : public RenderPassNode {
  public:
    JFAPass(
        std::string_view id,
        const vax::rhi::Device& device,
        vax::rhi::PipelineManager& pipelineManager,
        vax::rhi::TextureManager& textureManager
    ) noexcept
        : RenderPassNode(id)
        , _device(device)
        , _pipelineManager(pipelineManager)
        , _textureManager(textureManager) {};

    ~JFAPass() = default;

    JFAPass(const JFAPass& other) = delete;
    JFAPass& operator=(const JFAPass& other) = delete;
    JFAPass(JFAPass&& other) noexcept = default;
    JFAPass& operator=(JFAPass&& other) noexcept = default;

    void setup(std::weak_ptr<vax::rhi::RenderDestination> inputRenderDestination);

    void update(std::weak_ptr<vax::rhi::RenderDestination> inputRenderDestination);

    const SampledImageShaderHandle& outputImage(RunPassInfo& runPassInfo) const {
        if (_isFinalImageA) {
            return _jfaTexturesA[runPassInfo.frameIndex]->sampledShaderHandle;
        } else {
            return _jfaTexturesB[runPassInfo.frameIndex]->sampledShaderHandle;
        }
    }

    void runPass(RunPassInfo& runPassInfo) override;

  private:
    vax::Logger _logger = vax::Logger("JFAPass");
    std::reference_wrapper<const vax::rhi::Device> _device;
    std::reference_wrapper<const vax::rhi::PipelineManager> _pipelineManager;
    std::reference_wrapper<vax::rhi::TextureManager> _textureManager;
    std::optional<vax::rhi::Pipeline> _initPipeline;
    std::optional<vax::rhi::Pipeline> _stepPipeline;
    std::vector<vax::rhi::TextureManager::TextureResource*> _inputMaskTextures;
    const vax::rhi::TextureManager::TextureResource* _inputDepthTextures;
    std::vector<vax::rhi::TextureManager::TextureResource*> _jfaTexturesA;
    std::vector<vax::rhi::TextureManager::TextureResource*> _jfaTexturesB;
    std::weak_ptr<vax::rhi::RenderDestination> _inputRenderDestination;
    bool _isFinalImageA = true;

    void _createTextures(
        const std::vector<vax::rhi::TextureManager::TextureResource*>& maskTextures,
        const vax::rhi::TextureManager::TextureResource* depthTexture
    );
};
} // namespace vax::gfx