#include "indirectDrawController.h"

using namespace vax::gfx;
using namespace vax::vk;
using namespace vax;

void IndirectDrawController::setup(uint32_t maxCommands, vk::DescriptorRegistry& descriptorRegistry) noexcept {
    _maxCommands = maxCommands;
    _commands.reserve(maxCommands);
    for (uint32_t i = 0; i < vax::vk::MAX_FRAMES_IN_FLIGHT; i++) {
        auto buffer = IndirectDrawCommandBuffer::allocate(
            _device.get(),
            "indirect_draw_commands",
            _maxCommands * sizeof(VkDrawIndexedIndirectCommand),
            VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
            VMA_MEMORY_USAGE_CPU_TO_GPU
        );
        if (!buffer) {
            _logger.error("Failed to allocate indirect draw command buffer");
            return;
        }
        _commandBuffers.push_back(std::make_unique<IndirectDrawCommandBuffer>(std::move(*buffer)));

        _submitted.push_back(false);

        auto perDrawDataBuffer = PerDrawDataBuffer::allocate(
            _device.get(),
            "per_draw_data",
            _maxCommands * sizeof(PerDrawData),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_CPU_TO_GPU
        );
        if (!perDrawDataBuffer) {
            _logger.error("Failed to allocate per draw data buffer");
            return;
        }
        auto handle = descriptorRegistry.registerStorageBuffer(*perDrawDataBuffer);
        if (!handle) {
            _logger.error("Failed to register per draw data buffer");
            return;
        }
        _perDrawDataBufferHandles.push_back(
            std::make_unique<PerDrawDataBufferHandle>(std::move(handle.value()), std::move(*perDrawDataBuffer))
        );
    }
}

void IndirectDrawController::draw(CommandBuffer& commandBuffer, uint32_t frameIndex) noexcept {
    if (!_submitted[frameIndex]) {
        _logger.error("Commands not submitted");
        return;
    }
    vkCmdDrawIndexedIndirect(
        commandBuffer.vkCommandBuffer,
        _commandBuffers[frameIndex]->vkBuffer(),
        0,
        static_cast<uint32_t>(_commands.size()),
        sizeof(VkDrawIndexedIndirectCommand)
    );
}

void IndirectDrawController::drawRange(
    CommandBuffer& commandBuffer, uint32_t frameIndex, const std::string& name
) noexcept {
    auto& drawRange = _drawRanges.at(name);
    if (!_submitted[frameIndex]) {
        _logger.error("Commands not submitted");
        return;
    }
    vkCmdDrawIndexedIndirect(
        commandBuffer.vkCommandBuffer,
        _commandBuffers[frameIndex]->vkBuffer(),
        drawRange.start * sizeof(VkDrawIndexedIndirectCommand),
        drawRange.count,
        sizeof(VkDrawIndexedIndirectCommand)
    );
}

void IndirectDrawController::pushCommand(
    VkDrawIndexedIndirectCommand command, const PerDrawData& perDrawData
) noexcept {
    if (_commands.size() >= _maxCommands) {
        _logger.error("Max commands reached");
        return;
    }
    _commands.push_back(command);
    _perDrawData.push_back(perDrawData);
}

void IndirectDrawController::submitCommands(uint32_t frameIndex) noexcept {
    if (_commands.empty()) {
        return;
    }
    _commandBuffers[frameIndex]->fill(_commands.data(), _commands.size() * sizeof(VkDrawIndexedIndirectCommand));
    _perDrawDataBufferHandles[frameIndex]->second.fill(_perDrawData.data(), _perDrawData.size() * sizeof(PerDrawData));
    _submitted[frameIndex] = true;
}

void IndirectDrawController::prepareForDraw(uint32_t frameIndex) noexcept {
    _commands.clear();
    _perDrawData.clear();
    _submitted[frameIndex] = false;
}