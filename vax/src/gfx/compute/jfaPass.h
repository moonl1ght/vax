#pragma once

#include "pipelineManager.h"
#include "renderDestination.h"
#include "renderPassNode.h"
#include "texture.h"

namespace vax::gfx {
class JFAPass final : public RenderPassNode {
  public:
    JFAPass(std::string_view id, const vax::rhi::Device& device, vax::rhi::PipelineManager& pipelineManager) noexcept
        : RenderPassNode(id)
        , _device(device)
        , _pipelineManager(pipelineManager) {
        _textureFactory = std::make_unique<vax::rhi::TextureFactory>(device, pipelineManager.descriptorRegistry());
    };

    ~JFAPass() = default;

    JFAPass(const JFAPass& other) = delete;
    JFAPass& operator=(const JFAPass& other) = delete;
    JFAPass(JFAPass&& other) noexcept = default;
    JFAPass& operator=(JFAPass&& other) noexcept = default;

    void setup(std::weak_ptr<vax::rhi::RenderDestination> inputRenderDestination);

    void update(std::weak_ptr<vax::rhi::RenderDestination> inputRenderDestination);

    SampledImageShaderHandle outputImage(RunPassInfo& runPassInfo) const {
        if (_isFinalImageA) {
            return _jfaTexturesA[runPassInfo.frameIndex]->sampledShaderHandle();
        } else {
            return _jfaTexturesB[runPassInfo.frameIndex]->sampledShaderHandle();
        }
    }

    void runPass(RunPassInfo& runPassInfo) override;

  private:
    dev::Logger _logger = dev::Logger("JFAPass");
    std::reference_wrapper<const vax::rhi::Device> _device;
    std::reference_wrapper<const vax::rhi::PipelineManager> _pipelineManager;
    std::unique_ptr<vax::rhi::TextureFactory> _textureFactory;
    std::optional<vax::rhi::Pipeline> _initPipeline;
    std::optional<vax::rhi::Pipeline> _stepPipeline;
    const std::vector<std::unique_ptr<vax::rhi::Texture>>* _inputMaskTextures;
    vax::rhi::Texture* _inputDepthTextures;
    std::vector<std::unique_ptr<vax::rhi::Texture>> _jfaTexturesA;
    std::vector<std::unique_ptr<vax::rhi::Texture>> _jfaTexturesB;
    std::weak_ptr<vax::rhi::RenderDestination> _inputRenderDestination;
    bool _isFinalImageA = true;

    void _createTextures(
        const std::vector<std::unique_ptr<vax::rhi::Texture>>& maskTextures,
        const std::unique_ptr<vax::rhi::Texture>& depthTexture
    );
};
} // namespace vax::gfx