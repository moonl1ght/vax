#pragma once

#include <memory>
#include <unordered_map>

#include "renderDestination.h"
#include "renderPassDescriptor.h"
#include "renderPassNode.h"
#include "uiEngine.h"

namespace vax::gfx {

class RenderPassGraph {
  public:
    RenderPassGraph(std::unique_ptr<RenderPassNode> rootNode, editor::UIEngine& uiEngine)
        : _rootNode(std::move(rootNode))
        , _uiEngine(uiEngine) {};

    RenderPassGraph(const RenderPassGraph&) = delete;
    RenderPassGraph(RenderPassGraph&&) = default;
    RenderPassGraph& operator=(const RenderPassGraph&) = delete;
    RenderPassGraph& operator=(RenderPassGraph&&) = default;

    ~RenderPassGraph() = default;

    void run(RenderPassNode::RunPassInfo& runPassInfo);

    RenderPassNode* findNode(const std::string& name) const;

    void setNodeEnabled(const std::string& name, bool enabled);

  private:
    std::reference_wrapper<editor::UIEngine> _uiEngine;

    std::unique_ptr<RenderPassNode> _rootNode;
};

} // namespace vax::gfx