#pragma once

#include "renderPassGraph.h"
#include "renderPassGraphDescriptor.h"

namespace vax::engine {
class RenderPassGraphBuilder final {
  public:
    RenderPassGraphBuilder(
        vax::vk::Device& device,
        vax::WindowController& windowController,
        vax::vk::PipelineManager& pipelineManager,
        vax::vk::DescriptorSetManager& descriptorSetManager,
        vax::ui::UIEngine& uiEngine
    )
        : _device(device)
        , _windowController(windowController)
        , _pipelineManager(pipelineManager)
        , _descriptorSetManager(descriptorSetManager)
        , _uiEngine(uiEngine) {}

    ~RenderPassGraphBuilder() = default;

    RenderPassGraphBuilder(const RenderPassGraphBuilder&) = delete;
    RenderPassGraphBuilder& operator=(const RenderPassGraphBuilder&) = delete;
    RenderPassGraphBuilder(RenderPassGraphBuilder&&) = delete;
    RenderPassGraphBuilder& operator=(RenderPassGraphBuilder&&) = delete;

    std::unique_ptr<RenderPassGraph> buildUIOnlyGraph(
        const std::unordered_map<std::string, std::shared_ptr<vax::vk::RenderPassDescriptor>>& renderPassDescriptors,
        const std::unordered_map<std::string, std::shared_ptr<vax::vk::RenderDestination>>& renderDestinations
    );

    std::unique_ptr<RenderPassGraph> buildSceneGraph(
        const RenderPassGraphDescriptor& graphDescriptor,
        const std::unordered_map<std::string, std::shared_ptr<vax::vk::RenderPassDescriptor>>& renderPassDescriptors,
        const std::unordered_map<std::string, std::shared_ptr<vax::vk::RenderDestination>>& renderDestinations
    );

  private:
    std::reference_wrapper<vax::vk::Device> _device;
    std::reference_wrapper<vax::WindowController> _windowController;
    std::reference_wrapper<vax::vk::PipelineManager> _pipelineManager;
    std::reference_wrapper<vax::vk::DescriptorSetManager> _descriptorSetManager;
    std::reference_wrapper<vax::ui::UIEngine> _uiEngine;
};
} // namespace vax::engine