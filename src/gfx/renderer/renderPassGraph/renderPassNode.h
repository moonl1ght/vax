#pragma once

#include "commandBuffer.h"
#include "scene.h"
#include "sceneDrawData.h"
#include "shaderResourceHandle.h"
#include <string>

namespace vax::gfx {
class RenderPassNode {
  public:
    struct FrameTextureHandles final {
        SampledImageShaderHandle mainColor;
        SampledImageShaderHandle mainMask;
        SampledImageShaderHandle mainDepth;
        SampledImageShaderHandle agentCameraColor;
    };

    struct RunPassInfo final {
        vax::rhi::CommandBuffer& commandBuffer;
        vax::gfx::Scene* scene;
        std::vector<uint32_t> imageIndices;
        uint32_t frameIndex;
        FrameTextureHandles frameTextures{};
        SceneDrawData* sceneDrawData = nullptr;
    };

    RenderPassNode* prev = nullptr;
    std::unique_ptr<RenderPassNode> next = nullptr;

    RenderPassNode(std::string_view id)
        : _id(id) {};

    RenderPassNode(const RenderPassNode&) = delete;
    RenderPassNode(RenderPassNode&&) = default;
    RenderPassNode& operator=(const RenderPassNode&) = delete;
    RenderPassNode& operator=(RenderPassNode&&) = default;

    virtual ~RenderPassNode() = default;

    virtual void execute(RunPassInfo& runPassInfo) final {
        if (isEnabled()) {
            runPass(runPassInfo);
        }
        if (next) {
            next->execute(runPassInfo);
        }
    }

    void setEnabled(bool enabled) { _enabled = enabled; }

    bool isEnabled() const { return _enabled; }

    virtual void runPass(RunPassInfo& runPassInfo) = 0;

    std::string id() const { return _id; }

  protected:
    std::string _id;
    bool _enabled = true;
};
} // namespace vax::gfx