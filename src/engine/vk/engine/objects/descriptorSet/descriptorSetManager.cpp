#include "descriptorSetManager.h"
#include "descriptorSetLayoutBuilder.h"
#include "vkUtils.h"
#include <vector>
#include <vulkan/vulkan_core.h>

using namespace vax::vk;
using namespace vax;

void DescriptorSetManager::setup() noexcept {
    auto defaultPoolDescriptor = PoolDescriptor::defaultPoolDescriptor();
    _createDescriptorPool(defaultPoolDescriptor);
    auto uabPoolDescriptor = PoolDescriptor::defaultPoolDescriptor();
    uabPoolDescriptor.type = PoolType::UAB;
    _createDescriptorPool(uabPoolDescriptor);
}

void DescriptorSetManager::_createDescriptorPool(const PoolDescriptor& poolDescriptor) noexcept {
    VkDescriptorPoolCreateFlags flags = 0;
    bool isUAB = false;
    if (poolDescriptor.type == PoolType::UAB && poolDescriptor.poolSupportsUpdateAfterBind()) {
        flags |= VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
        isUAB = true;
    }
    VkDescriptorPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = flags,
        .maxSets = poolDescriptor.maxSets,
        .poolSizeCount = static_cast<uint32_t>(poolDescriptor.poolSizes.size()),
        .pPoolSizes = poolDescriptor.poolSizes.data(),
    };
    VkDescriptorPool descriptorPool;
    if (!VK_CHECK(vkCreateDescriptorPool(_device.get().vkDevice, &poolInfo, nullptr, &descriptorPool))) {
        _logger.error("Failed to create descriptor pool!");
        return;
    }
    if (isUAB) {
        _uabDescriptorPools.push_back(descriptorPool);
    } else {
        _descriptorPools.push_back(descriptorPool);
    }
}

void DescriptorSetManager::cleanupV2() noexcept {
    for (VkDescriptorPool pool : _descriptorPools) {
        vkDestroyDescriptorPool(_device.get().vkDevice, pool, nullptr);
    }
    for (VkDescriptorPool pool : _uabDescriptorPools) {
        vkDestroyDescriptorPool(_device.get().vkDevice, pool, nullptr);
    }
}

std::optional<DescriptorSetManager::GraphicsModuleLayouts>
DescriptorSetManager::addGraphicsModuleLayouts(const ShaderLibrary::GraphicsModule& graphicsModule) noexcept {
    auto vertexLayouts = _addLayoutsForFunction(*graphicsModule.vertexFunction);
    auto fragmentLayouts = _addLayoutsForFunction(*graphicsModule.fragmentFunction);
    if (!vertexLayouts || !fragmentLayouts) {
        return std::nullopt;
    }
    return GraphicsModuleLayouts{
        .vertexLayouts = std::move(*vertexLayouts),
        .fragmentLayouts = std::move(*fragmentLayouts),
    };
}

std::optional<DescriptorSetManager::ComputeModuleLayouts>
DescriptorSetManager::addComputeModuleLayouts(const ShaderLibrary::ComputeModule& computeModule) noexcept {
    auto computeLayouts = _addLayoutsForFunction(*computeModule.computeFunction);
    if (!computeLayouts) {
        return std::nullopt;
    }
    return ComputeModuleLayouts{.computeLayouts = std::move(*computeLayouts)};
}

std::optional<std::vector<DescriptorSetLayout*>>
DescriptorSetManager::_addLayoutsForFunction(const ShaderLibrary::ShaderFunction& function) noexcept {
    const auto& sets = function.reflectionData.sets;
    std::vector<DescriptorSetLayout*> descriptorSetLayouts;
    descriptorSetLayouts.reserve(sets.size());
    for (const auto& set : sets) {
        if (set.set != descriptorSetLayouts.size()) {
            _logger.error("Set ", set.set, " is not in the correct position for ", function.name);
            return std::nullopt;
        }
        DescriptorSetLayout* layout = _getOrCreateSetLayout(set);
        if (!layout) {
            _logger.error("Failed to create descriptor set layout for set ", set.set, " in ", function.name);
            return std::nullopt;
        }
        descriptorSetLayouts.push_back(layout);
    }
    return descriptorSetLayouts;
}

DescriptorSetLayout*
DescriptorSetManager::_getOrCreateSetLayout(const ShaderReflectionData::SetReflectionData& set) noexcept {
    if (auto it = _descriptorSetLayoutsV2.find(set); it != _descriptorSetLayoutsV2.end()) {
        return &it->second;
    }
    auto layoutBuilder = DescriptorSetLayoutBuilder(_device.get(), set.name);
    for (const auto& binding : set.bindings) {
        layoutBuilder.addBinding(binding);
    }
    auto layout = layoutBuilder.build();
    if (!layout) {
        return nullptr;
    }
    auto [it, inserted] = _descriptorSetLayoutsV2.try_emplace(set, _device.get(), layout.value(), set.name);
    _descriptorSetNameLayoutReflectionDataMap.insert({set.name, set});
    return &it->second;
}

std::vector<DescriptorSetWriter> DescriptorSetManager::getDescriptorSetWriters(const std::string& name) noexcept {
    auto descriptorSets = _createOrFindDescriptorSets(name);
    if (descriptorSets.empty()) {
        return {};
    }
    std::vector<DescriptorSetWriter> descriptorSetWriters;
    descriptorSetWriters.reserve(static_cast<size_t>(_maxFramesInFlight));
    for (const auto& descriptorSet : descriptorSets) {
        descriptorSetWriters.emplace_back(_device.get(), descriptorSet);
    }
    return descriptorSetWriters;
}

std::vector<DescriptorSetHandler> DescriptorSetManager::getDescriptorSetHandlers(const std::string& name) noexcept {
    auto descriptorSets = _createOrFindDescriptorSets(name);
    if (descriptorSets.empty()) {
        return {};
    }
    std::vector<DescriptorSetHandler> descriptorSetHandlers;
    descriptorSetHandlers.reserve(descriptorSets.size());
    for (const auto& descriptorSet : descriptorSets) {
        descriptorSetHandlers.emplace_back(DescriptorSetHandler(descriptorSet));
    }
    return descriptorSetHandlers;
}

std::vector<VkDescriptorSet> DescriptorSetManager::_createOrFindDescriptorSets(const std::string& name) noexcept {
    auto it = _descriptorSets.find(name);
    if (it == _descriptorSets.end()) {
        auto descriptorSets = _createDescriptorSets(name);
        if (descriptorSets.empty()) {
            return {};
        }
        it = _descriptorSets.emplace(name, std::move(descriptorSets)).first;
    }
    return it->second;
}

std::vector<VkDescriptorSet> DescriptorSetManager::_createDescriptorSets(const std::string& name) noexcept {
    std::vector<VkDescriptorSet> descriptorSets;
    auto layoutKey = _descriptorSetNameLayoutReflectionDataMap.find(name);
    if (layoutKey == _descriptorSetNameLayoutReflectionDataMap.end()) {
        return {};
    }
    auto setLayout = _descriptorSetLayoutsV2.find(layoutKey->second);
    if (setLayout == _descriptorSetLayoutsV2.end()) {
        return {};
    }
    std::vector<VkDescriptorSetLayout> layouts(
        static_cast<size_t>(_maxFramesInFlight), setLayout->second.getVkDescriptorSetLayout()
    );
    VkDescriptorPool descriptorPool =
        layoutKey->second.isUpdateAfterBind ? _uabDescriptorPools.back() : _descriptorPools.back();
    VkDescriptorSetAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(_maxFramesInFlight),
        .pSetLayouts = layouts.data(),
    };
    descriptorSets.resize(_maxFramesInFlight, VK_NULL_HANDLE);
    auto result = vkAllocateDescriptorSets(_device.get().vkDevice, &allocInfo, descriptorSets.data());
    if (result != VK_SUCCESS) {
        // TODO: Handle new pool creation
        _logger.error("Failed to allocate descriptor set!");
        return {};
    }
    return descriptorSets;
}

// LEGACY

const DescriptorSetLayout*
DescriptorSetManager::getDescriptorSetLayout(const DescriptorSetLayoutName& setLayoutName) const {
    auto it = _descriptorSetLayouts.find(getSetLayoutName(setLayoutName));
    if (it == _descriptorSetLayouts.end()) {
        return nullptr;
    }
    return &it->second;
}

std::optional<DescriptorSetWriter>
DescriptorSetManager::getDescriptorSetWriter(const CommonDescriptorSetName& setName, uint32_t frameIndex) {
    auto it = _descriptorSets.find(getCommonDescriptorSetName(setName));
    if (it == _descriptorSets.end()) {
        auto descriptorSets = _createDescriptorSets(setName);
        if (descriptorSets.empty()) {
            return std::nullopt;
        }
        _descriptorSets[getCommonDescriptorSetName(setName)] = descriptorSets;
        return std::make_optional<DescriptorSetWriter>(_device.get(), descriptorSets[frameIndex]);
    }
    return std::make_optional<DescriptorSetWriter>(_device.get(), it->second[frameIndex]);
}

std::optional<DescriptorSetHandler>
DescriptorSetManager::getDescriptorSetHandler(const CommonDescriptorSetName& setName, uint32_t frameIndex) const {
    auto it = _descriptorSets.find(getCommonDescriptorSetName(setName));
    if (it == _descriptorSets.end()) {
        return std::nullopt;
    }
    return std::make_optional<DescriptorSetHandler>(it->second[frameIndex]);
}

void DescriptorSetManager::cleanup() {
    vkDestroyDescriptorPool(_device.get().vkDevice, _persistentDescriptorPool, nullptr);
    vkDestroyDescriptorPool(_device.get().vkDevice, _persistentUABDescriptorPool, nullptr);
}

bool DescriptorSetManager::setup(std::vector<DescriptorSetInfo> descriptorSetInfos) {
    for (const auto& descriptorSetInfo : descriptorSetInfos) {
        _descriptorSetInfos.insert({getCommonDescriptorSetName(descriptorSetInfo.name), descriptorSetInfo});
    }
    return _createDescriptorSetLayouts(descriptorSetInfos) && _createDescriptorSetPools(descriptorSetInfos);
}

std::vector<VkDescriptorSet> DescriptorSetManager::_createDescriptorSets(const CommonDescriptorSetName& setName) {
    auto setNameStr = getCommonDescriptorSetName(setName);
    auto it = _descriptorSetInfos.find(setNameStr);
    if (it == _descriptorSetInfos.end()) {
        return {};
    }

    auto setLayoutName = getSetLayoutName(it->second.layoutName);
    auto setLayoutIt = _descriptorSetLayouts.find(setLayoutName);
    if (setLayoutIt == _descriptorSetLayouts.end()) {
        return {};
    }

    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<VkDescriptorSetLayout> layouts(
        static_cast<size_t>(_maxFramesInFlight), setLayoutIt->second.getVkDescriptorSetLayout()
    );
    VkDescriptorPool descriptorPool = it->second.isUABPool ? _persistentUABDescriptorPool : _persistentDescriptorPool;
    VkDescriptorSetAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(_maxFramesInFlight),
        .pSetLayouts = layouts.data(),
    };
    descriptorSets.resize(_maxFramesInFlight, VK_NULL_HANDLE);
    auto result = vkAllocateDescriptorSets(_device.get().vkDevice, &allocInfo, descriptorSets.data());
    if (result != VK_SUCCESS) {
        _logger.error("Failed to allocate descriptor set!");
        return {};
    }
    return descriptorSets;
}

bool DescriptorSetManager::_createDescriptorSetLayouts(const std::vector<DescriptorSetInfo>& descriptorSetInfos) {
    for (const auto& descriptorSetInfo : descriptorSetInfos) {
        auto descriptorSetLayoutInfo = getDescriptorLayoutResources(descriptorSetInfo.layoutName);
        auto descriptorSetLayoutName = getSetLayoutName(descriptorSetInfo.layoutName);
        DescriptorSetLayoutBuilder builder(_device.get(), descriptorSetLayoutName);
        for (const auto& descriptorLayoutResourceInfo : descriptorSetLayoutInfo) {
            VkDescriptorSetLayoutBinding binding{
                .binding = descriptorLayoutResourceInfo.binding,
                .descriptorType = descriptorLayoutResourceInfo.type,
                .descriptorCount = descriptorLayoutResourceInfo.descriptorCount,
                .stageFlags = descriptorLayoutResourceInfo.stageFlags,
                .pImmutableSamplers = nullptr,
            };
            builder.addBinding(binding);
        }
        auto layout =
            builder.build(descriptorSetInfo.isUABPool ? VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT : 0);
        if (!layout) {
            _logger.error("Failed to create descriptor set layout!");
            return false;
        }
        _descriptorSetLayouts.try_emplace(
            descriptorSetLayoutName, _device.get(), layout.value(), descriptorSetLayoutName
        );
    }
    return true;
}

bool DescriptorSetManager::_createDescriptorSetPools(const std::vector<DescriptorSetInfo>& descriptorSetInfos) {
    std::unordered_map<VkDescriptorType, uint32_t> poolSizes;
    std::unordered_map<VkDescriptorType, uint32_t> uabPoolSizes;
    uint32_t maxAllocatedSets = 0;
    uint32_t maxUABAllocatedSets = 0;
    for (const auto& descriptorSetInfo : descriptorSetInfos) {
        auto resourceRequirements = getDescriptorLayoutResources(descriptorSetInfo.layoutName);
        for (const auto& resourceRequirement : resourceRequirements) {
            if (descriptorSetInfo.isUABPool) {
                uabPoolSizes[resourceRequirement.type] += resourceRequirement.descriptorCount * _maxFramesInFlight;
            } else {
                poolSizes[resourceRequirement.type] += resourceRequirement.descriptorCount * _maxFramesInFlight;
            }
        }
        if (descriptorSetInfo.isUABPool) {
            ++maxUABAllocatedSets;
        } else {
            ++maxAllocatedSets;
        }
    }

    std::vector<VkDescriptorPoolSize> poolSizesVector;
    std::vector<VkDescriptorPoolSize> uabPoolSizesVector;

    for (const auto& [type, count] : poolSizes) {
        poolSizesVector.push_back({type, count});
    }
    for (const auto& [type, count] : uabPoolSizes) {
        uabPoolSizesVector.push_back({type, count});
    }

    if (maxAllocatedSets > 0) {
        VkDescriptorPoolCreateInfo poolInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = 0,
            .maxSets = static_cast<uint32_t>(_maxFramesInFlight) * maxAllocatedSets,
            .poolSizeCount = static_cast<uint32_t>(poolSizesVector.size()),
            .pPoolSizes = poolSizesVector.data(),
        };
        if (!VK_CHECK(vkCreateDescriptorPool(_device.get().vkDevice, &poolInfo, nullptr, &_persistentDescriptorPool))) {
            _logger.error("Failed to create persistent descriptor pool!");
            return false;
        }
    }

    if (maxUABAllocatedSets > 0) {
        VkDescriptorPoolCreateInfo poolInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
            .maxSets = static_cast<uint32_t>(_maxFramesInFlight) * maxUABAllocatedSets,
            .poolSizeCount = static_cast<uint32_t>(uabPoolSizesVector.size()),
            .pPoolSizes = uabPoolSizesVector.data(),
        };
        if (!VK_CHECK(
                vkCreateDescriptorPool(_device.get().vkDevice, &poolInfo, nullptr, &_persistentUABDescriptorPool)
            )) {
            _logger.error("Failed to create persistent UAB descriptor pool!");
            return false;
        }
    }
    return true;
}