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

    struct VkContext final {
        VkBuffer indexBuffer = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkDescriptorSet uboDescriptorSet = VK_NULL_HANDLE;
        uint32_t frameUniformStride = 0;
    };

    SceneDrawData(IndirectDrawController& indirectDrawController, Handles handles, VkContext vkContext)
        : _indirectDrawController(indirectDrawController)
        , _handles(handles)
        , _vkContext(vkContext) {}

    template <typename MakePushConstants>
    void drawRange(
        const DrawContext& drawContext,
        const std::string& name,
        std::optional<FrameUniformSlot> slot,
        MakePushConstants&& makePushConstants
    ) const noexcept {
        auto range = _indirectDrawController.getDrawRange(name);
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
    VkContext _vkContext;

    void _bindIndexBuffer(const DrawContext& drawContext) const noexcept;

    void _bindFrameUniformSlot(const DrawContext& drawContext, FrameUniformSlot slot) const noexcept;

    void _pushConstants(const DrawContext& drawContext, const void* data, uint32_t size) const noexcept;
};

} // namespace vax::gfx
