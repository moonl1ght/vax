#pragma once

#include "buffer.h"
#include "commandBuffer.h"
#include "descriptorRegistry.h"
#include "device.h"
#include "logger.h"
#include "shaderData.h"
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace vax::gfx {

class IndirectDrawController final {
  public:
    using IndirectDrawCommandBuffer = vk::Buffer<VkDrawIndexedIndirectCommand>;
    using PerDrawDataBuffer = vk::Buffer<PerDrawData>;
    using PerDrawDataBufferHandle = vk::ShaderBufferHandle<PerDrawData>;

    struct DrawRange final {
        uint32_t start;
        uint32_t count;
    };

    explicit IndirectDrawController(const vk::Device& device) noexcept
        : _device(device) {};

    ~IndirectDrawController() = default;

    void setup(uint32_t maxCommands, vk::DescriptorRegistry& descriptorRegistry) noexcept;

    void prepareForDraw(uint32_t frameIndex) noexcept;

    void pushCommand(VkDrawIndexedIndirectCommand command, const PerDrawData& perDrawData) noexcept;

    template <typename Function> void addDrawScope(const std::string& name, Function function) noexcept {
        auto start = _commands.size();
        function();
        auto count = _commands.size() - start;
        _drawRanges[name] = DrawRange{.start = static_cast<uint32_t>(start), .count = static_cast<uint32_t>(count)};
    }

    void submitCommands(uint32_t frameIndex) noexcept;

    void draw(vk::CommandBuffer& commandBuffer, uint32_t frameIndex) noexcept;

    void drawRange(vk::CommandBuffer& commandBuffer, uint32_t frameIndex, const std::string& name) noexcept;

    std::optional<DrawRange> drawRange(const std::string& name) const noexcept {
        if (auto it = _drawRanges.find(name); it != _drawRanges.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    StructuredBufferShaderHandle perDrawDataHandle(uint32_t frameIndex) const noexcept {
        return _perDrawDataBufferHandles[frameIndex]->first;
    }

  private:
    vax::Logger _logger = vax::Logger("IndirectDrawController");

    std::reference_wrapper<const vk::Device> _device;

    std::vector<VkDrawIndexedIndirectCommand> _commands;

    std::vector<PerDrawData> _perDrawData;

    std::vector<std::unique_ptr<IndirectDrawCommandBuffer>> _commandBuffers;

    std::vector<std::unique_ptr<PerDrawDataBufferHandle>> _perDrawDataBufferHandles;

    std::unordered_map<std::string, DrawRange> _drawRanges;

    uint32_t _maxCommands;

    std::vector<bool> _submitted;
};

} // namespace vax::gfx