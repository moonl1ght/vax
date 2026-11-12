#pragma once

#include "commonDescriptorSets.h"
#include "descriptorSetHandler.h"
#include "descriptorSetLayout.h"
#include "descriptorSetWriter.h"
#include "device.h"
#include "poolDescriptor.h"
#include "reflectionProcessor.h"
#include "shaderLibrary.h"
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace vax::vk {
class DescriptorSetManager final {
  public:
    struct PipelineSetLayouts final {
        std::vector<DescriptorSetLayout*> setLayouts;
        std::optional<VkPushConstantRange> pushConstants;
    };

    explicit DescriptorSetManager(const vax::vk::Device& device, const int32_t maxFramesInFlight)
        : _device(device)
        , _maxFramesInFlight(maxFramesInFlight) {};

    ~DescriptorSetManager() { cleanupV2(); };

    DescriptorSetManager(const DescriptorSetManager&) = delete;
    DescriptorSetManager& operator=(const DescriptorSetManager&) = delete;
    DescriptorSetManager(DescriptorSetManager&& other) = delete;
    DescriptorSetManager& operator=(DescriptorSetManager&& other) = delete;

    void setup() noexcept;

    void cleanupV2() noexcept;

    std::optional<PipelineSetLayouts>
    addPipelineModuleLayouts(const ShaderLibrary::PipelineModule& pipelineModule) noexcept;

    std::vector<DescriptorSetWriter> getDescriptorSetWriters(const std::string& name) noexcept;

    std::vector<DescriptorSetHandler> getDescriptorSetHandlers(const std::string& name) noexcept;

    // LEGACY

    bool setup(std::vector<DescriptorSetInfo> descriptorSetInfos);

    void cleanup();

    const DescriptorSetLayout* getDescriptorSetLayout(const DescriptorSetLayoutName& setLayoutName) const;

    std::optional<DescriptorSetWriter>
    getDescriptorSetWriter(const CommonDescriptorSetName& setName, uint32_t frameIndex);

    std::optional<DescriptorSetHandler>
    getDescriptorSetHandler(const CommonDescriptorSetName& setName, uint32_t frameIndex) const;

  private:
    vax::Logger _logger = vax::Logger("DescriptorSetManager");
    std::reference_wrapper<const vax::vk::Device> _device;

    const int32_t _maxFramesInFlight;

    std::vector<VkDescriptorPool> _uabDescriptorPools;
    std::vector<VkDescriptorPool> _descriptorPools;

    std::unordered_map<ShaderReflectionData::SetReflectionData, DescriptorSetLayout> _descriptorSetLayoutsV2;
    std::unordered_map<std::string, std::vector<VkDescriptorSet>> _descriptorSets;
    std::unordered_map<std::string, ShaderReflectionData::SetReflectionData> _descriptorSetNameLayoutReflectionDataMap;

    DescriptorSetLayout* _getOrCreateSetLayout(const ShaderReflectionData::SetReflectionData& set) noexcept;

    // void _createDescriptorPool(const PoolDescriptor& poolDescriptor) noexcept;

    std::vector<VkDescriptorSet> _createDescriptorSets(const std::string& name) noexcept;

    std::vector<VkDescriptorSet> _createOrFindDescriptorSets(const std::string& name) noexcept;

    // LEGACY

    std::unordered_map<std::string, DescriptorSetInfo> _descriptorSetInfos;
    std::unordered_map<std::string, DescriptorSetLayout> _descriptorSetLayouts;

    VkDescriptorPool _persistentDescriptorPool = VK_NULL_HANDLE;
    VkDescriptorPool _persistentUABDescriptorPool = VK_NULL_HANDLE;

    bool _createDescriptorSetPools(const std::vector<DescriptorSetInfo>& descriptorSetInfos);

    bool _createDescriptorSetLayouts(const std::vector<DescriptorSetInfo>& descriptorSetInfos);

    std::vector<VkDescriptorSet> _createDescriptorSets(const CommonDescriptorSetName& setName);
};
} // namespace vax::vk
