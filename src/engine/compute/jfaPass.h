#pragma once

#include "pipelineManager.h"
#include "renderDestination.h"
#include "renderPassNode.h"
#include "texture.h"

namespace vax::engine {
class JFAPass final : public RenderPassNode {
  public:
    JFAPass(std::string_view id, const vax::vk::Device& device, vax::vk::PipelineManager& pipelineManager) noexcept
        : RenderPassNode(id)
        , _device(device)
        , _pipelineManager(pipelineManager) {};

    ~JFAPass() = default;

    JFAPass(const JFAPass& other) = delete;
    JFAPass& operator=(const JFAPass& other) = delete;
    JFAPass(JFAPass&& other) noexcept = default;
    JFAPass& operator=(JFAPass&& other) noexcept = default;

    void setup(std::weak_ptr<vax::vk::RenderDestination> inputRenderDestination);

    void update(std::weak_ptr<vax::vk::RenderDestination> inputRenderDestination);

    const std::vector<vax::vk::Texture>& outputATextures() const;

    const std::vector<vax::vk::Texture>& outputBTextures() const;

    const StorageImageShaderHandle& outputImage(RunPassInfo& runPassInfo) const {
        if (_isFinalImageA) {
            return _textureHandles[runPassInfo.frameIndex].jfaA;
        } else {
            return _textureHandles[runPassInfo.frameIndex].jfaB;
        }
    }

    const SampledImageShaderHandle& outputSampledImage(uint32_t frameIndex) const {
        const auto& handles = _textureHandles[frameIndex];
        return _isFinalImageA ? handles.jfaASampled : handles.jfaBSampled;
    }

    void runPass(RunPassInfo& runPassInfo) override;

  private:
    struct TextureHandles final {
        SampledImageShaderHandle mask;
        SampledImageShaderHandle depth;
        StorageImageShaderHandle jfaA;
        StorageImageShaderHandle jfaB;
        SampledImageShaderHandle jfaASampled;
        SampledImageShaderHandle jfaBSampled;
    };

    vax::Logger _logger = vax::Logger("JFAPass");
    std::reference_wrapper<const vax::vk::Device> _device;
    std::reference_wrapper<const vax::vk::PipelineManager> _pipelineManager;
    std::optional<vax::vk::Pipeline> _initPipeline;
    std::optional<vax::vk::Pipeline> _stepPipeline;
    std::vector<vax::vk::Texture> _jfaTexturesA;
    std::vector<vax::vk::Texture> _jfaTexturesB;
    std::weak_ptr<vax::vk::RenderDestination> _inputRenderDestination;
    bool _isFinalImageA = true;
    std::vector<TextureHandles> _textureHandles;

    void _createTextures(const std::vector<vax::vk::Texture>& maskTextures, const vax::vk::Texture& depthTexture);
};
} // namespace vax::engine