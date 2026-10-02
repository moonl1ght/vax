#include "primitivesBuilder.h"

using namespace vax::engine;
using namespace vax::vk;
using namespace vax;

std::optional<DrawableModel> PrimitivesBuilder::createCube(float size, vax::engine::Color color) {
    auto mesh = _resourceManager.get().meshManager().createEmptyMesh();
    if (!mesh)
        return std::nullopt;
    float s = size / 2.0f;
    (*mesh).second->setVertices(
        {// Front face (Z+)
         {{-s, -s, s}, 0, {0, 0, 1, 0}, {0, 0, 0}, 0, {0, 0}, {0, 0}},
         {{s, -s, s}, 0, {0, 0, 1, 0}, {0, 0, 0}, 0, {1, 0}, {0, 0}},
         {{s, s, s}, 0, {0, 0, 1, 0}, {0, 0, 0}, 0,{1, 1}, {0, 0}},
         {{-s, s, s}, 0, {0, 0, 1, 0}, {0, 0, 0}, 0,{0, 1}, {0, 0}},
         // Back face (Z-)
         {{-s, -s, -s}, 0, {0, 0, -1, 0}, {0, 0, 0}, 0,{1, 0}, {0, 0}},
         {{-s, s, -s}, 0, {0, 0, -1, 0}, {0, 0, 0}, 0,{1, 1}, {0, 0}},
         {{s, s, -s}, 0, {0, 0, -1, 0}, {0, 0, 0}, 0,{0, 1}, {0, 0}},
         {{s, -s, -s}, 0, {0, 0, -1, 0}, {0, 0, 0}, 0,{0, 0}, {0, 0}},
         // Top face (Y+)
         {{-s, s, -s}, 0, {0, 1, 0, 0}, {0, 0, 0}, 0,{0, 1}, {0, 0}},
         {{-s, s, s}, 0, {0, 1, 0, 0}, {0, 0, 0}, 0,{0, 0}, {0, 0}},
         {{s, s, s}, 0, {0, 1, 0, 0}, {0, 0, 0}, 0,{1, 0}, {0, 0}},
         {{s, s, -s}, 0, {0, 1, 0, 0}, {0, 0, 0}, 0,{1, 1}, {0, 0}},
         // Bottom face (Y-)
         {{-s, -s, -s}, 0, {0, -1, 0, 0}, {0, 0, 0}, 0,{0, 0}, {0, 0}},
         {{s, -s, -s}, 0, {0, -1, 0, 0}, {0, 0, 0}, 0,{1, 0}, {0, 0}},
         {{s, -s, s}, 0, {0, -1, 0, 0}, {0, 0, 0}, 0,{1, 1}, {0, 0}},
         {{-s, -s, s}, 0, {0, -1, 0, 0}, {0, 0, 0}, 0,{0, 1}, {0, 0}},
         // Right face (X+)
         {{s, -s, -s}, 0, {1, 0, 0, 0}, {0, 0, 0}, 0,{1, 0}, {0, 0}},
         {{s, s, -s}, 0, {1, 0, 0, 0}, {0, 0, 0}, 0,{1, 1}, {0, 0}},
         {{s, s, s}, 0, {1, 0, 0, 0}, {0, 0, 0}, 0,{0, 1}, {0, 0}},
         {{s, -s, s}, 0, {1, 0, 0, 0}, {0, 0, 0}, 0,{0, 0}, {0, 0}},
         // Left face (X-)
         {{-s, -s, -s}, 0, {-1, 0, 0, 0}, {0, 0, 0}, 0,{0, 0}, {0, 0}},
         {{-s, -s, s}, 0, {-1, 0, 0, 0}, {0, 0, 0}, 0,{1, 0}, {0, 0}},
         {{-s, s, s}, 0, {-1, 0, 0, 0}, {0, 0, 0}, 0,{1, 1}, {0, 0}},
         {{-s, s, -s}, 0, {-1, 0, 0, 0}, {0, 0, 0}, 0,{0, 1}, {0, 0}}
        }
    );
    for (int i = 0; i < 6; ++i) {
        unsigned int offset = i * 4;
        mesh.value().second->addIndex(offset + 0);
        mesh.value().second->addIndex(offset + 1);
        mesh.value().second->addIndex(offset + 2);

        mesh.value().second->addIndex(offset + 0);
        mesh.value().second->addIndex(offset + 2);
        mesh.value().second->addIndex(offset + 3);
    }

    mesh.value().second->lock();
    mesh.value().second->bindBuffers();

    if (!mesh.value().second->flushToGPU()) {
        return std::nullopt;
    }

    PBRMaterial material{
        .baseColor = color,
    };
    material.baseColorTextureIndex = NO_TEXTURE_FLAG;
    auto materialIndex = _resourceManager.get().materialManager().insert(material);
    Submesh submesh{
        .indexCount = static_cast<uint32_t>(mesh.value().second->indexCount()),
        .materialIndex = materialIndex,
    };
    auto drawableModel = vax::engine::DrawableModel(_resourceManager.get().meshManager(), mesh.value().first);
    drawableModel._mesh = mesh.value().second;
    drawableModel._submeshes.push_back(submesh);
    return std::optional<DrawableModel>(std::in_place, std::move(drawableModel));
}

std::optional<DrawableModel> PrimitivesBuilder::createPlane() {
    auto mesh = _resourceManager.get().meshManager().createEmptyMesh();
    if (!mesh)
        return std::nullopt;
    mesh.value().second->setVertices({
        {{-1.0f, -1.0f, 1.0f}, 0, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 0, {0.0f, 0.0f}, {0.0f, 0.0f}},
        {{-1.0f, 1.0f, 1.0f}, 0, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 0, {0.0f, 1.0f}, {0.0f, 0.0f}},
        {{1.0f, -1.0f, 1.0f}, 0, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 0, {1.0f, 0.0f}, {0.0f, 0.0f}},
        {{1.0f, 1.0f, 1.0f}, 0, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 0, {1.0f, 1.0f}, {0.0f, 0.0f}},
    });

    mesh.value().second->addIndex(0);
    mesh.value().second->addIndex(2);
    mesh.value().second->addIndex(1);
    mesh.value().second->addIndex(1);
    mesh.value().second->addIndex(2);
    mesh.value().second->addIndex(3);

    mesh.value().second->lock();
    mesh.value().second->bindBuffers();
    if (!mesh.value().second->flushToGPU()) {
        return std::nullopt;
    }
    Submesh submesh{
        .indexCount = static_cast<uint32_t>(mesh.value().second->indexCount()),
        .materialIndex = NO_MATERIAL_INDEX,
    };
    auto drawableModel = vax::engine::DrawableModel(_resourceManager.get().meshManager(), mesh.value().first);
    drawableModel._mesh = mesh.value().second;
    drawableModel._submeshes.push_back(submesh);
    drawableModel.setSettings(
        DrawableModel::Settings{
        .skipPushConstants = true,
        }
    );
    return std::optional<DrawableModel>(std::in_place, std::move(drawableModel));
}