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
    struct GraphDescriptor {
        bool withGizmo;
        bool withRoverCamera;
    };

    RenderPassGraph(
        std::shared_ptr<RenderPassNode> rootNode,
        std::unordered_map<std::string, std::shared_ptr<vax::vk::RenderPassDescriptor>> renderPassDescriptors,
        std::unordered_map<std::string, std::shared_ptr<vax::vk::RenderDestination>> renderDestinations,
        vax::ui::UIEngine& uiEngine
    )
        : _renderPassDescriptors(renderPassDescriptors)
        , _renderDestinations(renderDestinations)
        , _rootNode(rootNode)
        , _uiEngine(uiEngine) {};

    RenderPassGraph(const RenderPassGraph&) = delete;
    RenderPassGraph(RenderPassGraph&&) = default;
    RenderPassGraph& operator=(const RenderPassGraph&) = delete;
    RenderPassGraph& operator=(RenderPassGraph&&) = default;

    ~RenderPassGraph() = default;

    void run(RenderPassNode::RunPassInfo& runPassInfo);

    std::shared_ptr<vax::vk::RenderDestination> getRenderDestination(const std::string& name) const {
        if (!_renderDestinations.contains(name)) {
            return nullptr;
        }
        return _renderDestinations.at(name);
    }

    std::shared_ptr<vax::vk::RenderPassDescriptor> getRenderPassDescriptor(const std::string& name) const {
        if (!_renderPassDescriptors.contains(name)) {
            return nullptr;
        }
        return _renderPassDescriptors.at(name);
    }

    std::shared_ptr<RenderPassNode> findNode(const std::string& name) const;

    void setNodeEnabled(const std::string& name, bool enabled);

  private:
    std::reference_wrapper<vax::ui::UIEngine> _uiEngine;
    std::unordered_map<std::string, std::shared_ptr<vax::vk::RenderPassDescriptor>> _renderPassDescriptors;
    std::unordered_map<std::string, std::shared_ptr<vax::vk::RenderDestination>> _renderDestinations;

    std::shared_ptr<RenderPassNode> _rootNode = nullptr;
};

} // namespace vax::engine