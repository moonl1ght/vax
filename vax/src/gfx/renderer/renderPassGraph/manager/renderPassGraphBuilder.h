#pragma once

#include "pipelineManager.h"
#include "renderPassGraph.h"
#include "renderPassGraphDescriptor.h"

namespace vax::gfx {
class RenderPassGraphBuilder final {
  public:
    RenderPassGraphBuilder(
        vax::vk::Device& device,
        vax::WindowController& windowController,
        vax::vk::PipelineManager& pipelineManager,
        vax::ui::UIEngine& uiEngine,
        vax::vk::ResourceManager& resourceManager
    )
        : _device(device)
        , _windowController(windowController)
        , _pipelineManager(pipelineManager)
        , _uiEngine(uiEngine)
        , _resourceManager(resourceManager) {}

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
    std::reference_wrapper<vax::ui::UIEngine> _uiEngine;
    std::reference_wrapper<vax::vk::ResourceManager> _resourceManager;
};
} // namespace vax::gfx