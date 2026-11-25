#pragma once

#include "drawContext.h"
#include "frameUniformSlot.h"
#include "indirectDrawController.h"
#include "shaderResourceHandle.h"
#include <optional>
#include <string>

namespace vax::gfx {

struct SceneDrawData final {
    using DrawRange = IndirectDrawController::DrawRange;

    struct Handles final {
        StructuredBufferShaderHandle vertices;
        StructuredBufferShaderHandle instances;
        StructuredBufferShaderHandle drawData;
        StructuredBufferShaderHandle materials;
    };

    IndirectDrawController& indirectDrawController;
    Handles handles;
    VkBuffer indexBuffer = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSet uboDescriptorSet = VK_NULL_HANDLE;
    uint32_t frameUniformStride = 0;

    SceneDrawData(IndirectDrawController& indirectDrawController)
        : indirectDrawController(indirectDrawController) {}

    template <typename MakePushConstants>
    void drawRange(
        const DrawContext& drawContext,
        const std::string& name,
        std::optional<FrameUniformSlot> slot,
        MakePushConstants&& makePushConstants
    ) const noexcept {
        auto range = indirectDrawController.drawRange(name);
        if (!range) {
            return;
        }
        _bindIndexBuffer(drawContext);
        if (slot) {
            _bindFrameUniformSlot(drawContext, *slot);
        }
        const auto pushConstants = makePushConstants(*range, handles);
        _pushConstants(drawContext, &pushConstants, sizeof(pushConstants));
        indirectDrawController.drawRange(drawContext.commandBuffer, drawContext.currentFrame, name);
    }

  private:
    void _bindIndexBuffer(const DrawContext& drawContext) const noexcept;

    void _bindFrameUniformSlot(const DrawContext& drawContext, FrameUniformSlot slot) const noexcept;

    void _pushConstants(const DrawContext& drawContext, const void* data, uint32_t size) const noexcept;
};

} // namespace vax::gfx
