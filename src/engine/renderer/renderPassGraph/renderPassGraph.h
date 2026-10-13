#pragma once

#include <memory>
#include <unordered_map>

#include "renderDestination.h"
#include "renderPassDescriptor.h"
#include "renderPassNode.h"
#include "uiEngine.h"

namespace vax::engine {

class RenderPassGraph {
  public:
    RenderPassGraph(std::shared_ptr<RenderPassNode> rootNode, vax::ui::UIEngine& uiEngine)
        : _rootNode(rootNode)
        , _uiEngine(uiEngine) {};

    RenderPassGraph(const RenderPassGraph&) = delete;
    RenderPassGraph(RenderPassGraph&&) = default;
    RenderPassGraph& operator=(const RenderPassGraph&) = delete;
    RenderPassGraph& operator=(RenderPassGraph&&) = default;

    ~RenderPassGraph() = default;

    void run(RenderPassNode::RunPassInfo& runPassInfo);

    std::shared_ptr<RenderPassNode> findNode(const std::string& name) const;

    void setNodeEnabled(const std::string& name, bool enabled);

  private:
    std::reference_wrapper<vax::ui::UIEngine> _uiEngine;

    std::shared_ptr<RenderPassNode> _rootNode = nullptr;
};

} // namespace vax::engine