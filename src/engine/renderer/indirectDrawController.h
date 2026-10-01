#pragma once

#include "buffer.h"
#include "commandBuffer.h"
#include "descriptorSetWriter.h"
#include "device.h"
#include "logger.h"
#include "luna.h"
#include "shaderUniforms.h"
#include <unordered_map>

namespace vax::engine {
class IndirectDrawController final {
  public:
    using IndirectDrawCommandBuffer = vk::Buffer<VkDrawIndexedIndirectCommand>;
    using PerDrawDataBuffer = vk::Buffer<PerDrawData>;

    struct DrawRange final {
        uint32_t start;
        uint32_t count;
    };

    IndirectDrawController(const vk::Device& device)
        : _device(device) {};

    ~IndirectDrawController() = default;

    void setup(uint32_t maxCommands);

    void writePerDrawDescriptorSet(vax::vk::DescriptorSetWriter& descriptorWriter, uint32_t frameIndex);

    void prepareForDraw(uint32_t frameIndex);

    void pushCommand(VkDrawIndexedIndirectCommand command, const PerDrawData& perDrawData);

    template <typename Function> void addDrawScope(const std::string& name, Function function) {
        auto start = _commands.size();
        function();
        auto count = _commands.size() - start;
        _drawRanges[name] = DrawRange{.start = static_cast<uint32_t>(start), .count = static_cast<uint32_t>(count)};
    }

    void submitCommands(uint32_t frameIndex);

    void draw(vk::CommandBuffer& commandBuffer, uint32_t frameIndex);

    void drawRange(vk::CommandBuffer& commandBuffer, uint32_t frameIndex, const std::string& name);

    std::optional<DrawRange> drawRange(const std::string& name) const {
      if (auto it = _drawRanges.find(name); it != _drawRanges.end()) {
        return it->second;
      }
      return std::nullopt;
    }

  private:
    vax::Logger _logger = vax::Logger("IndirectDrawController");

    std::reference_wrapper<const vk::Device> _device;

    std::vector<VkDrawIndexedIndirectCommand> _commands;

    std::vector<PerDrawData> _perDrawData;

    std::vector<std::unique_ptr<IndirectDrawCommandBuffer>> _commandBuffers;

    std::vector<std::unique_ptr<PerDrawDataBuffer>> _perDrawDataBuffers;

    std::unordered_map<std::string, DrawRange> _drawRanges;

    uint32_t _maxCommands;

    std::vector<bool> _submitted;
};
} // namespace vax::engine