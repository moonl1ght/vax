#include "pipelineManager.h"
#include "pipelineBuilder.h"
#include <optional>

using namespace vax::rhi;
using namespace vax;

bool PipelineManager::setup() noexcept {
    _shaderLibrary = std::make_unique<ShaderLibrary>();
    _descriptorManager = std::make_unique<DescriptorManager>(_device.get());
    if (!_descriptorManager->setup()) {
        return false;
    }
    return true;
}

bool PipelineManager::load(const std::string& shaderDirectory) noexcept {
    if (_isLoaded) {
        _logger.error("Pipeline manager already loaded!");
        return false;
    }
    _shaderLibrary->startSession(shaderDirectory);
    _isLoaded = true;
    return true;
}

void PipelineManager::unload() noexcept {
    _shaderLibrary->endSession();
    _isLoaded = false;
}

void PipelineManager::_destroy() noexcept {
    for (auto& [name, pipeline] : _pipelines) {
        vkDestroyPipeline(_device.get().vkDevice(), pipeline.vkPipeline, nullptr);
    }
    _pipelines.clear();
    vkDestroyPipelineLayout(_device.get().vkDevice(), _pipelineLayout, nullptr);
}

bool PipelineManager::prebuildPipelines(const std::vector<PipelineDescriptor>& pipelineDescriptors) noexcept {
    if (!_isLoaded) {
        _logger.error("Pipeline manager not loaded!");
        return false;
    }

    ShaderModuleBuilder shaderModuleBuilder(_device.get());
    PipelineLayoutBuilder pipelineLayoutBuilder(_device.get());
    pipelineLayoutBuilder.addDescriptorSetLayout(_descriptorManager->descriptorSetLayout());    // BINDLESS_SET_INDEX
    pipelineLayoutBuilder.addDescriptorSetLayout(_descriptorManager->uboDescriptorSetLayout()); // UBO_SET_INDEX
    VkPipelineLayout layout = pipelineLayoutBuilder.build("bindlessPipelineLayout");
    if (layout == VK_NULL_HANDLE) {
        _logger.error("Failed to create bindless pipeline layout!");
        return false;
    }
    _pipelineLayout = layout;
    for (const PipelineDescriptor& pipelineDescriptor : pipelineDescriptors) {
        if (_pipelines.contains(pipelineDescriptor.name)) {
            _logger.warning("Pipeline already built, skipping: ", pipelineDescriptor.name);
            continue;
        }
        auto pipelineModule = _shaderLibrary->loadPipelineModule(pipelineDescriptor.shaderModuleDescriptor);
        if (!pipelineModule || pipelineModule->functions.empty()) [[unlikely]] {
            _logger.error("Failed to load pipeline module: ", pipelineDescriptor.shaderModuleDescriptor.name);
            continue;
        }
        switch (pipelineDescriptor.type) {
        case PipelineDescriptor::Type::COMPUTE: {
            if (pipelineModule->functions.size() != 1) {
                _logger.error("Compute pipeline must have exactly one function: ", pipelineDescriptor.name);
                break;
            }
            ComputePipelineBuilder pipelineBuilder(_device.get());
            pipelineBuilder.setPipelineLayout(_pipelineLayout);
            auto shaderModule = shaderModuleBuilder.build(pipelineModule->functions[0].spirv);
            if (!shaderModule) {
                _logger.error("Failed to create compute shader module!");
                break;
            }
            pipelineBuilder.setShaderModule(shaderModule.value(), "main");
            auto pipeline = pipelineBuilder.build(pipelineDescriptor.name);
            if (!pipeline) {
                _logger.error("Failed to create compute pipeline!");
                break;
            }
            _pipelines.emplace(
                pipelineDescriptor.name,
                Pipeline{
                .vkPipeline = pipeline.value(),
                .pipelineLayout = _pipelineLayout,
                .descriptorSetLayout = _descriptorManager->descriptorSet(),
                }
            );
            break;
        }
        case PipelineDescriptor::Type::GRAPHICS: {
            if (pipelineDescriptor.renderPassDescriptor == nullptr) {
                _logger.error("Graphics pipeline has no render pass descriptor: ", pipelineDescriptor.name);
                break;
            }
            GraphicsPipelineBuilder pipelineBuilder(_device.get());
            std::vector<ShaderModule> shaderModules;
            shaderModules.reserve(pipelineModule->functions.size());
            bool shaderModulesBuilt = true;
            for (const auto& function : pipelineModule->functions) {
                auto shaderModule = shaderModuleBuilder.build(function.spirv);
                if (!shaderModule) {
                    _logger.error("Failed to create shader module for function: ", function.name);
                    shaderModulesBuilt = false;
                    break;
                }
                shaderModules.push_back(std::move(*shaderModule));
                auto stageFlags = static_cast<VkShaderStageFlagBits>(function.stageFlags);
                pipelineBuilder.addShaderStage(stageFlags, shaderModules.back().vkModule, "main");
            }
            if (!shaderModulesBuilt) {
                break;
            }
            pipelineBuilder.setDepthStencilState(pipelineDescriptor.depthStencilState);
            pipelineBuilder.setRenderPass(pipelineDescriptor.renderPassDescriptor);
            auto pipeline = pipelineBuilder.build(pipelineDescriptor.name, _pipelineLayout);
            if (!pipeline) {
                _logger.error("Failed to create graphics pipeline!");
                break;
            }
            _pipelines.emplace(
                pipelineDescriptor.name,
                Pipeline{
                .vkPipeline = pipeline.value(),
                .pipelineLayout = _pipelineLayout,
                .descriptorSetLayout = _descriptorManager->descriptorSet(),
                }
            );
            break;
        }
        }
    }
    return true;
}