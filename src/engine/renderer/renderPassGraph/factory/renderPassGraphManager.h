#pragma once

#include "commandManager.h"
#include "descriptorSetManager.h"
#include "device.h"
#include "logger.h"
#include "pipelineManager.h"
#include "queueManager.h"
#include "renderDestination.h"
#include "renderPassGraph.h"
#include "swapchain.h"
#include "uiEngine.h"
#include "windowController.h"
#include <unordered_map>

namespace vax::engine {
class RenderPassGraphManager final {
  public:
    enum class GraphMode { SCENE_GRAPH, UI_GRAPH };

    RenderPassGraphManager(
        vax::vk::Device& device,
        vax::vk::PipelineManager& pipelineManager,
        vax::vk::DescriptorSetManager& descriptorSetManager,
        vax::WindowController& windowController,
        vax::ui::UIEngine& uiEngine
    )
        : _device(device)
        , _pipelineManager(pipelineManager)
        , _descriptorSetManager(descriptorSetManager)
        , _windowController(windowController)
        , _uiEngine(uiEngine) {};

    ~RenderPassGraphManager() = default;

    void setupRenderPassDescriptors(VkFormat imageFormat);

    void setupRenderDestinations(
        vax::vk::CommandManager& commandManager, vax::vk::QueueManager& queueManager, vax::vk::Swapchain& swapchain
    );

    void setupRenderDestinationsForRoverCamera(
        vax::vk::CommandManager& commandManager, vax::vk::QueueManager& queueManager, vax::vk::Swapchain& swapchain
    );

    void buildGraph();

    std::shared_ptr<vax::vk::RenderPassDescriptor> getRenderPassDescriptor(std::string name) const {
        if (_renderPassDescriptors.contains(name)) {
            return _renderPassDescriptors.at(name);
        }
        return nullptr;
    }

    const RenderPassGraph& getRenderPassGraph() const { return *_renderPassGraph; }

    RenderPassGraph& getRenderPassGraph() { return *_renderPassGraph; }

    void setGraphMode(GraphMode graphMode) { _graphMode = graphMode; }

    void setGraphDescriptor(RenderPassGraph::GraphDescriptor graphDescriptor) { _graphDescriptor = graphDescriptor; }

    template <typename Updater> void updateGraphDescriptor(Updater&& updater) { updater(_graphDescriptor.value()); }

  private:
    vax::Logger _logger = vax::Logger("rednerPassGraphFactory");

    GraphMode _graphMode = GraphMode::UI_GRAPH;
    std::optional<RenderPassGraph::GraphDescriptor> _graphDescriptor = std::nullopt;

    std::reference_wrapper<vax::vk::Device> _device;
    std::reference_wrapper<vax::WindowController> _windowController;
    std::reference_wrapper<vax::vk::PipelineManager> _pipelineManager;
    std::reference_wrapper<vax::vk::DescriptorSetManager> _descriptorSetManager;
    std::reference_wrapper<vax::ui::UIEngine> _uiEngine;

    std::unordered_map<std::string, std::shared_ptr<vax::vk::RenderPassDescriptor>> _renderPassDescriptors;
    std::unordered_map<std::string, std::shared_ptr<vax::vk::RenderDestination>> _renderDestinations;

    std::unique_ptr<RenderPassGraph> _renderPassGraph;

    void _buildUIOnlyGraph();

    void _buildSceneGraph(RenderPassGraph::GraphDescriptor& graphDescriptor);
};
} // namespace vax::engine