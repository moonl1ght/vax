#pragma once

#include "commonDescriptorSets.h"
#include "descriptorSetHandler.h"
#include "descriptorSetLayout.h"
#include "descriptorSetWriter.h"
#include "device.h"
#include "luna.h"
#include "poolDescriptor.h"
#include <unordered_map>

namespace vax::vk {
class DescriptorSetManager final {
  public:
    explicit DescriptorSetManager(const vax::vk::Device& device, const int32_t maxFramesInFlight)
        : _device(device)
        , _maxFramesInFlight(maxFramesInFlight) {};

    ~DescriptorSetManager() {};

    DescriptorSetManager(const DescriptorSetManager&) = delete;
    DescriptorSetManager& operator=(const DescriptorSetManager&) = delete;
    DescriptorSetManager(DescriptorSetManager&& other) = delete;
    DescriptorSetManager& operator=(DescriptorSetManager&& other) = delete;

    void setup() noexcept;

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

    std::vector<VkDescriptorPool> _uabDescriptorPools;
    std::vector<VkDescriptorPool> _descriptorPools;

    void _createDescriptorPool(const PoolDescriptor& poolDescriptor);

    // LEGACY
    const int32_t _maxFramesInFlight;

    std::unordered_map<std::string_view, DescriptorSetInfo> _descriptorSetInfos;
    std::unordered_map<std::string_view, DescriptorSetLayout> _descriptorSetLayouts;
    std::unordered_map<std::string_view, std::vector<VkDescriptorSet>> _descriptorSets;

    VkDescriptorPool _persistentDescriptorPool = VK_NULL_HANDLE;
    VkDescriptorPool _persistentUABDescriptorPool = VK_NULL_HANDLE;

    bool _createDescriptorSetPools(const std::vector<DescriptorSetInfo>& descriptorSetInfos);

    bool _createDescriptorSetLayouts(const std::vector<DescriptorSetInfo>& descriptorSetInfos);

    std::vector<VkDescriptorSet> _createDescriptorSets(const CommonDescriptorSetName& setName);
};
} // namespace vax::vk
