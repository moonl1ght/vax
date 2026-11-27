#include "sceneDrawData.h"
#include "shaderConstants.h"

using namespace vax::gfx;

void SceneDrawData::_bindIndexBuffer(const DrawContext& drawContext) const noexcept {
    vkCmdBindIndexBuffer(drawContext.commandBuffer.vkCommandBuffer, _vkContext.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
}

void SceneDrawData::_bindFrameUniformSlot(const DrawContext& drawContext, FrameUniformSlot slot) const noexcept {
    uint32_t dynamicOffset = static_cast<uint32_t>(slot) * _vkContext.frameUniformStride;
    vkCmdBindDescriptorSets(
        drawContext.commandBuffer.vkCommandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        _vkContext.pipelineLayout,
        UBO_SET_INDEX,
        1,
        &_vkContext.uboDescriptorSet,
        1,
        &dynamicOffset
    );
}

void SceneDrawData::_pushConstants(const DrawContext& drawContext, const void* data, uint32_t size) const noexcept {
    vkCmdPushConstants(
        drawContext.commandBuffer.vkCommandBuffer, _vkContext.pipelineLayout, VK_SHADER_STAGE_ALL, 0, size, data
    );
}
