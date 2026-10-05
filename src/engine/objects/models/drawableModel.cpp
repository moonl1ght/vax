#include "drawableModel.h"

using namespace vax::engine;
using namespace vax::vk;

void DrawableModel::prepareDrawing(
    engine::IndirectDrawController* indirectDrawController, uint32_t frameIndex, const DrawSettings& drawSettings
) {
    uint32_t flags = ObjectFlags::NoFlags;
    if (_settings.useWireframe) {
        flags |= ObjectFlags::IsWireframe;
    }
    if (!_settings.hasTangents) {
        flags |= ObjectFlags::NoTangent;
    }
    if (_settings.precomputedMVP) {
        flags |= ObjectFlags::PrecomputedMVP;
    }

    for (auto& submesh : _submeshes) {
        VkDrawIndexedIndirectCommand drawIndexedIndirectCommand{
            .indexCount = submesh.indexCount,
            .instanceCount = drawSettings.instancesCount,
            .firstIndex = _mesh->globalMemoryIndexCursor().offset + submesh.firstIndex,
            .vertexOffset = static_cast<int32_t>(_mesh->globalMemoryVertexCursor().offset + submesh.vertexOffset),
            .firstInstance = drawSettings.instanceOffset,
        };
        PerDrawData perDrawData{
            .flags = flags,
            .materialIndex = submesh.materialIndex,
        };
        indirectDrawController->pushCommand(drawIndexedIndirectCommand, perDrawData);
    }
}