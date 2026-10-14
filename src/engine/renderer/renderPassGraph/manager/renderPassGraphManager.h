#pragma once

#include "commandManager.h"
#include "descriptorSetManager.h"
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

namespace vax::engine {
class RenderPassGraphManager final {
  public:
    RenderPassGraphManager(
        vax::vk::Device& device,
        vax::vk::PipelineManager& pipelineManager,
        vax::vk::DescriptorSetManager& descriptorSetManager,
        vax::WindowController& windowController,
        vax::ui::UIEngine& uiEngine
    )
        : _device(device)
        , _pipelineManager(pipelineManager)
        , _descriptorSetManager(descriptorSetManager) {
        _renderPassGraphBuilder = std::make_unique<RenderPassGraphBuilder>(
            _device, windowController, _pipelineManager, _descriptorSetManager, uiEngine
        );
    }

    ~RenderPassGraphManager() = default;

    void setupRenderPassDescriptors(VkFormat imageFormat);

    void setupRenderDestinations(
        vax::vk::CommandManager& commandManager, vax::vk::QueueManager& queueManager, vax::vk::Swapchain& swapchain
    );

    void setupRenderDestinationsForAgentCamera(
        vax::vk::CommandManager& commandManager, vax::vk::QueueManager& queueManager, vax::vk::Swapchain& swapchain
    );

    void buildGraph(const RenderPassGraphMode& graphMode, const RenderPassGraphDescriptor* graphDescriptor);

    vax::vk::RenderPassDescriptor* getRenderPassDescriptor(std::string name) const {
        if (_renderPassDescriptors.contains(name)) {
            return _renderPassDescriptors.at(name).get();
        }
        return nullptr;
    }

    vax::vk::RenderDestination* getRenderDestination(std::string name) const {
        if (_renderDestinations.contains(name)) {
            return _renderDestinations.at(name).get();
        }
        return nullptr;
    }

    const RenderPassGraph& getRenderPassGraph() const { return *_renderPassGraph; }

    RenderPassGraph& getRenderPassGraph() { return *_renderPassGraph; }

  private:
    vax::Logger _logger = vax::Logger("rednerPassGraphFactory");

    std::reference_wrapper<vax::vk::Device> _device;
    std::reference_wrapper<vax::vk::PipelineManager> _pipelineManager;
    std::reference_wrapper<vax::vk::DescriptorSetManager> _descriptorSetManager;

    std::unordered_map<std::string, std::shared_ptr<vax::vk::RenderPassDescriptor>> _renderPassDescriptors;
    std::unordered_map<std::string, std::shared_ptr<vax::vk::RenderDestination>> _renderDestinations;

    std::unique_ptr<RenderPassGraphBuilder> _renderPassGraphBuilder;

    std::unique_ptr<RenderPassGraph> _renderPassGraph;
};
} // namespace vax::engine