#pragma once

#include "commandManager.h"
#include "device.h"
#include "logger.h"
#include "pipelineManager.h"
#include "queueManager.h"
#include "renderDestination.h"
#include "renderPassGraph.h"
#include "renderPassGraphBuilder.h"
#include "swapchain.h"
#include "uiEngine.h"
#include "windowController.h"
#include <unordered_map>

namespace vax::gfx {
class RenderPassGraphManager final {
  public:
    RenderPassGraphManager(
        vax::rhi::Device& device,
        vax::rhi::PipelineManager& pipelineManager,
        vax::rhi::ResourceManager& resourceManager,
        app::WindowController& windowController,
        gfx::UIEngine& uiEngine
    )
        : _device(device)
        , _pipelineManager(pipelineManager)
        , _resourceManager(resourceManager) {
        _renderPassGraphBuilder = std::make_unique<RenderPassGraphBuilder>(
            _device, windowController, _pipelineManager, uiEngine, _resourceManager
        );
    }

    ~RenderPassGraphManager() = default;

    void setupRenderPassDescriptors(VkFormat imageFormat);

    void setupRenderDestinations(
        vax::rhi::CommandManager& commandManager, vax::rhi::QueueManager& queueManager, vax::rhi::Swapchain& swapchain
    );

    void setupRenderDestinationsForAgentCamera(
        vax::rhi::CommandManager& commandManager, vax::rhi::QueueManager& queueManager, vax::rhi::Swapchain& swapchain
    );

    void buildGraph(const RenderPassGraphMode& graphMode, const RenderPassGraphDescriptor* graphDescriptor);

    vax::rhi::RenderPassDescriptor* getRenderPassDescriptor(std::string name) const {
        if (_renderPassDescriptors.contains(name)) {
            return _renderPassDescriptors.at(name).get();
        }
        return nullptr;
    }

    vax::rhi::RenderDestination* getRenderDestination(std::string name) const {
        if (_renderDestinations.contains(name)) {
            return _renderDestinations.at(name).get();
        }
        return nullptr;
    }

    const RenderPassGraph& getRenderPassGraph() const { return *_renderPassGraph; }

    RenderPassGraph& getRenderPassGraph() { return *_renderPassGraph; }

  private:
    dev::Logger _logger = dev::Logger("rednerPassGraphFactory");

    std::reference_wrapper<vax::rhi::Device> _device;
    std::reference_wrapper<vax::rhi::PipelineManager> _pipelineManager;
    std::reference_wrapper<vax::rhi::ResourceManager> _resourceManager;

    std::unordered_map<std::string, std::shared_ptr<vax::rhi::RenderPassDescriptor>> _renderPassDescriptors;
    std::unordered_map<std::string, std::shared_ptr<vax::rhi::RenderDestination>> _renderDestinations;

    std::unique_ptr<RenderPassGraphBuilder> _renderPassGraphBuilder;

    std::unique_ptr<RenderPassGraph> _renderPassGraph;
};
} // namespace vax::gfx