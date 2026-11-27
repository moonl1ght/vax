#pragma once

#include "drawContext.h"
#include "editorScene.h"
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

    SceneDrawData(IndirectDrawController& indirectDrawController, const scene::EditorScene& scene)
        : _indirectDrawController(indirectDrawController) {}

    template <typename MakePushConstants>
    void drawRange(
        const DrawContext& drawContext,
        const std::string& name,
        std::optional<FrameUniformSlot> slot,
        MakePushConstants&& makePushConstants
    ) const noexcept {
        auto range = _indirectDrawController.drawRange(name);
        if (!range) [[unlikely]] {
            return;
        }
        _bindIndexBuffer(drawContext);
        if (slot) {
            _bindFrameUniformSlot(drawContext, *slot);
        }
        const auto pushConstants = makePushConstants(*range, _handles);
        _pushConstants(drawContext, &pushConstants, sizeof(pushConstants));
        _indirectDrawController.drawRange(drawContext.commandBuffer, drawContext.currentFrame, name);
    }

    const Handles& handles() const noexcept { return _handles; }

  private:
    IndirectDrawController& _indirectDrawController;
    Handles _handles;
    VkBuffer _indexBuffer = VK_NULL_HANDLE;
    VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSet _uboDescriptorSet = VK_NULL_HANDLE;
    uint32_t _frameUniformStride = 0;

    void _bindIndexBuffer(const DrawContext& drawContext) const noexcept;

    void _bindFrameUniformSlot(const DrawContext& drawContext, FrameUniformSlot slot) const noexcept;

    void _pushConstants(const DrawContext& drawContext, const void* data, uint32_t size) const noexcept;
};

} // namespace vax::gfx
