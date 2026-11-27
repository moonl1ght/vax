#pragma once

#include "renderPassNode.h"
#include <memory>

namespace vax::gfx {

class RenderPassGraph final {
  public:
    RenderPassGraph(std::unique_ptr<RenderPassNode> rootNode)
        : _rootNode(std::move(rootNode)) {};

    RenderPassGraph(const RenderPassGraph&) = delete;
    RenderPassGraph(RenderPassGraph&&) = default;
    RenderPassGraph& operator=(const RenderPassGraph&) = delete;
    RenderPassGraph& operator=(RenderPassGraph&&) = default;

    ~RenderPassGraph() = default;

    void run(RenderPassNode::RunPassInfo& runPassInfo);

    RenderPassNode* findNode(const std::string& name) const;

    void setNodeEnabled(const std::string& name, bool enabled);

  private:
    std::unique_ptr<RenderPassNode> _rootNode;
};

} // namespace vax::gfx