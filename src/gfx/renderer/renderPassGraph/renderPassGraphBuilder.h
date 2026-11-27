#pragma once

#include "pipelineManager.h"
#include "renderPass.h"
#include "renderPassGraph.h"
#include "renderPassGraphDescriptor.h"
#include "renderStep.h"
#include "renderTarget.h"
#include "uiEngine.h"

namespace vax::gfx {
class RenderPassGraphBuilder final {
  public:
    RenderPassGraphBuilder(
        vax::rhi::Device& device,
        editor::WindowController& windowController,
        vax::rhi::PipelineManager& pipelineManager,
        editor::UIEngine& uiEngine,
        vax::rhi::ResourceManager& resourceManager
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

    void prebuild(const gfx::RenderTarget& renderTarget);

    std::unique_ptr<RenderPassGraph> buildUIOnlyGraph(const gfx::RenderTarget& renderTarget);

    std::unique_ptr<RenderPassGraph> buildEditorSceneGraph(const gfx::RenderTarget& renderTarget);

    std::unique_ptr<RenderPassGraph> buildUIOnlyGraph(
        const std::unordered_map<std::string, std::shared_ptr<vax::rhi::RenderPassDescriptor>>& renderPassDescriptors,
        const std::unordered_map<std::string, std::shared_ptr<vax::rhi::RenderDestination>>& renderDestinations
    );

    std::unique_ptr<RenderPassGraph> buildSceneGraph(
        const RenderPassGraphDescriptor& graphDescriptor,
        const std::unordered_map<std::string, std::shared_ptr<vax::rhi::RenderPassDescriptor>>& renderPassDescriptors,
        const std::unordered_map<std::string, std::shared_ptr<vax::rhi::RenderDestination>>& renderDestinations
    );

  private:
    std::reference_wrapper<vax::rhi::Device> _device;
    std::reference_wrapper<editor::WindowController> _windowController;
    std::reference_wrapper<vax::rhi::PipelineManager> _pipelineManager;
    std::reference_wrapper<editor::UIEngine> _uiEngine;
    std::reference_wrapper<vax::rhi::ResourceManager> _resourceManager;

    std::unordered_map<std::string, std::shared_ptr<RenderStep>> _renderSteps;
};
} // namespace vax::gfx