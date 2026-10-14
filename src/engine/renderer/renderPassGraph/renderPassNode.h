#pragma once

#include <string>

#include "commandBuffer.h"
#include "scene.h"
#include "commonDescriptorSets.h"

namespace vax::engine {
class RenderPassNode {
  public:
    struct InputDescriptorSetInfo {
        vax::vk::CommonDescriptorSetName setName;

        struct BindingInfo {
            uint32_t setIndex;
            VkPipelineBindPoint bindPoint;
            uint32_t dynamicOffsetCount;
            std::vector<uint32_t> dynamicOffsets;
        };

        BindingInfo bindingInfo;
    };

    struct RunPassInfo final {
        vax::vk::CommandBuffer& commandBuffer;
        vax::engine::Scene* scene;
        std::vector<uint32_t> imageIndices;
        uint32_t frameIndex;
    };

    std::weak_ptr<RenderPassNode> prev;
    std::shared_ptr<RenderPassNode> next = nullptr;

    RenderPassNode(std::string_view id)
        : _id(id) {};

    RenderPassNode(const RenderPassNode&) = delete;
    RenderPassNode(RenderPassNode&&) = default;
    RenderPassNode& operator=(const RenderPassNode&) = delete;
    RenderPassNode& operator=(RenderPassNode&&) = default;

    virtual ~RenderPassNode() = default;

    virtual void execute(RunPassInfo& runPassInfo) final {
        if (_enabled) {
            if (_prePassWork) {
                _prePassWork(this, runPassInfo);
            }
            runPass(runPassInfo);
            if (_postPassWork) {
                _postPassWork(this, runPassInfo);
            }
        }
        if (next) {
            next->execute(runPassInfo);
        }
    }

    void setEnabled(bool enabled) { _enabled = enabled; }

    bool isEnabled() const { return _enabled; }

    virtual void runPass(RunPassInfo& runPassInfo) = 0;

    void setPrePassWork(std::function<void(RenderPassNode*, RunPassInfo&)> prePassWork) { _prePassWork = prePassWork; }

    void setPostPassWork(std::function<void(RenderPassNode*, RunPassInfo&)> postPassWork) {
        _postPassWork = postPassWork;
    }

    std::string id() const { return _id; }

  protected:
    std::function<void(RenderPassNode*, RunPassInfo&)> _prePassWork;
    std::function<void(RenderPassNode*, RunPassInfo&)> _postPassWork;
    std::string _id;
    bool _enabled = true;
};
} // namespace vax::engine