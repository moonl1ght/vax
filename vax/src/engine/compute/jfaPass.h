#pragma once

#include "pipelineManager.h"
#include "renderDestination.h"
#include "renderPassNode.h"
#include "texture.h"

namespace vax::engine {
class JFAPass final : public RenderPassNode {
  public:
    JFAPass(
        std::string_view id,
        const vax::vk::Device& device,
        vax::vk::PipelineManager& pipelineManager,
        vax::vk::TextureManager& textureManager
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

    void setup(std::weak_ptr<vax::vk::RenderDestination> inputRenderDestination);

    void update(std::weak_ptr<vax::vk::RenderDestination> inputRenderDestination);

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
    std::reference_wrapper<const vax::vk::Device> _device;
    std::reference_wrapper<const vax::vk::PipelineManager> _pipelineManager;
    std::reference_wrapper<vax::vk::TextureManager> _textureManager;
    std::optional<vax::vk::Pipeline> _initPipeline;
    std::optional<vax::vk::Pipeline> _stepPipeline;
    std::vector<vax::vk::TextureManager::TextureResource*> _inputMaskTextures;
    const vax::vk::TextureManager::TextureResource* _inputDepthTextures;
    std::vector<vax::vk::TextureManager::TextureResource*> _jfaTexturesA;
    std::vector<vax::vk::TextureManager::TextureResource*> _jfaTexturesB;
    std::weak_ptr<vax::vk::RenderDestination> _inputRenderDestination;
    bool _isFinalImageA = true;

    void _createTextures(
        const std::vector<vax::vk::TextureManager::TextureResource*>& maskTextures,
        const vax::vk::TextureManager::TextureResource* depthTexture
    );
};
} // namespace vax::engine