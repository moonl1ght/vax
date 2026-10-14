#include "primitivesBuilder.h"
#include <algorithm>
#include <unordered_map>

using namespace vax::engine;
using namespace vax::vk;
using namespace vax;

std::optional<DrawableModel>
PrimitivesBuilder::createCube(float width, float height, float depth, vax::engine::Color color) {
    auto mesh = _resourceManager.get().meshManager().createEmptyMesh();
    if (!mesh)
        return std::nullopt;
    float x = width / 2.0f;
    float y = height / 2.0f;
    float z = depth / 2.0f;
    (*mesh).second->setVertices(
        {// Front face (Z+)
         {{-x, -y, z}, 0, {0, 0, 1, 0}, {0, 0, 0}, 0, {0, 0}, {0, 0}},
         {{x, -y, z}, 0, {0, 0, 1, 0}, {0, 0, 0}, 0, {1, 0}, {0, 0}},
         {{x, y, z}, 0, {0, 0, 1, 0}, {0, 0, 0}, 0,{1, 1}, {0, 0}},
         {{-x, y, z}, 0, {0, 0, 1, 0}, {0, 0, 0}, 0,{0, 1}, {0, 0}},
         // Back face (Z-)
         {{-x, -y, -z}, 0, {0, 0, -1, 0}, {0, 0, 0}, 0,{1, 0}, {0, 0}},
         {{-x, y, -z}, 0, {0, 0, -1, 0}, {0, 0, 0}, 0,{1, 1}, {0, 0}},
         {{x, y, -z}, 0, {0, 0, -1, 0}, {0, 0, 0}, 0,{0, 1}, {0, 0}},
         {{x, -y, -z}, 0, {0, 0, -1, 0}, {0, 0, 0}, 0,{0, 0}, {0, 0}},
         // Top face (Y+)
         {{-x, y, -z}, 0, {0, 1, 0, 0}, {0, 0, 0}, 0,{0, 1}, {0, 0}},
         {{-x, y, z}, 0, {0, 1, 0, 0}, {0, 0, 0}, 0,{0, 0}, {0, 0}},
         {{x, y, z}, 0, {0, 1, 0, 0}, {0, 0, 0}, 0,{1, 0}, {0, 0}},
         {{x, y, -z}, 0, {0, 1, 0, 0}, {0, 0, 0}, 0,{1, 1}, {0, 0}},
         // Bottom face (Y-)
         {{-x, -y, -z}, 0, {0, -1, 0, 0}, {0, 0, 0}, 0,{0, 0}, {0, 0}},
         {{x, -y, -z}, 0, {0, -1, 0, 0}, {0, 0, 0}, 0,{1, 0}, {0, 0}},
         {{x, -y, z}, 0, {0, -1, 0, 0}, {0, 0, 0}, 0,{1, 1}, {0, 0}},
         {{-x, -y, z}, 0, {0, -1, 0, 0}, {0, 0, 0}, 0,{0, 1}, {0, 0}},
         // Right face (X+)
         {{x, -y, -z}, 0, {1, 0, 0, 0}, {0, 0, 0}, 0,{1, 0}, {0, 0}},
         {{x, y, -z}, 0, {1, 0, 0, 0}, {0, 0, 0}, 0,{1, 1}, {0, 0}},
         {{x, y, z}, 0, {1, 0, 0, 0}, {0, 0, 0}, 0,{0, 1}, {0, 0}},
         {{x, -y, z}, 0, {1, 0, 0, 0}, {0, 0, 0}, 0,{0, 0}, {0, 0}},
         // Left face (X-)
         {{-x, -y, -z}, 0, {-1, 0, 0, 0}, {0, 0, 0}, 0,{0, 0}, {0, 0}},
         {{-x, -y, z}, 0, {-1, 0, 0, 0}, {0, 0, 0}, 0,{1, 0}, {0, 0}},
         {{-x, y, z}, 0, {-1, 0, 0, 0}, {0, 0, 0}, 0,{1, 1}, {0, 0}},
         {{-x, y, -z}, 0, {-1, 0, 0, 0}, {0, 0, 0}, 0,{0, 1}, {0, 0}}
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

std::optional<DrawableModel> PrimitivesBuilder::createPlane(float width, float height, vax::engine::Color color) {
    auto mesh = _resourceManager.get().meshManager().createEmptyMesh();
    if (!mesh)
        return std::nullopt;
    float x = width / 2.0f;
    float y = height / 2.0f;
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

std::optional<DrawableModel>
PrimitivesBuilder::createSphere(float radius, uint32_t segments, uint32_t rings, vax::engine::Color color) {
    segments = std::max(segments, 3u);
    rings = std::max(rings, 2u);

    std::vector<Vertex> vertices;
    vertices.reserve((rings + 1) * (segments + 1));
    for (uint32_t ring = 0; ring <= rings; ++ring) {
        float v = static_cast<float>(ring) / rings;
        float phi = v * static_cast<float>(M_PI);
        for (uint32_t segment = 0; segment <= segments; ++segment) {
            float u = static_cast<float>(segment) / segments;
            float theta = u * 2.0f * static_cast<float>(M_PI);
            glm::vec3 normal = {sin(phi) * cos(theta), cos(phi), sin(phi) * sin(theta)};
            glm::vec4 tangent = {-sin(theta), 0.0f, cos(theta), 1.0f};
            vertices.push_back({normal * radius, 0, tangent, normal, 0, {u, v}, {0, 0}});
        }
    }

    std::vector<uint32_t> indices;
    indices.reserve(rings * segments * 6);
    for (uint32_t ring = 0; ring < rings; ++ring) {
        uint32_t k1 = ring * (segments + 1);
        uint32_t k2 = k1 + segments + 1;
        for (uint32_t segment = 0; segment < segments; ++segment, ++k1, ++k2) {
            if (ring != 0) {
                indices.insert(indices.end(), {k1, k1 + 1, k2});
            }
            if (ring != rings - 1) {
                indices.insert(indices.end(), {k1 + 1, k2 + 1, k2});
            }
        }
    }

    return _createModel(vertices, indices, color);
}

std::optional<DrawableModel>
PrimitivesBuilder::createIcosphere(float radius, uint32_t subdivisions, vax::engine::Color color) {
    subdivisions = std::min(subdivisions, 7u);

    const float t = (1.0f + std::sqrt(5.0f)) / 2.0f;
    std::vector<glm::vec3> positions = {
        {-1, t, 0},
        {1, t, 0},
        {-1, -t, 0},
        {1, -t, 0},
        {0, -1, t},
        {0, 1, t},
        {0, -1, -t},
        {0, 1, -t},
        {t, 0, -1},
        {t, 0, 1},
        {-t, 0, -1},
        {-t, 0, 1},
    };
    for (auto& position : positions) {
        position = glm::normalize(position);
    }

    std::vector<uint32_t> indices = {
        0, 11, 5, 0, 5, 1, 0, 1, 7, 0, 7, 10, 0, 10, 11, 1, 5, 9, 5, 11, 4,  11, 10, 2,  10, 7, 6, 7, 1, 8,
        3, 9,  4, 3, 4, 2, 3, 2, 6, 3, 6, 8,  3, 8,  9,  4, 9, 5, 2, 4,  11, 6,  2,  10, 8,  6, 7, 9, 8, 1,
    };

    for (uint32_t level = 0; level < subdivisions; ++level) {
        std::unordered_map<uint64_t, uint32_t> midpoints;
        auto midpoint = [&](uint32_t a, uint32_t b) {
            uint64_t key = (static_cast<uint64_t>(std::min(a, b)) << 32) | std::max(a, b);
            if (auto it = midpoints.find(key); it != midpoints.end()) {
                return it->second;
            }
            auto index = static_cast<uint32_t>(positions.size());
            positions.push_back(glm::normalize(positions[a] + positions[b]));
            midpoints.emplace(key, index);
            return index;
        };
        std::vector<uint32_t> subdivided;
        subdivided.reserve(indices.size() * 4);
        for (size_t i = 0; i < indices.size(); i += 3) {
            uint32_t a = indices[i];
            uint32_t b = indices[i + 1];
            uint32_t c = indices[i + 2];
            uint32_t ab = midpoint(a, b);
            uint32_t bc = midpoint(b, c);
            uint32_t ca = midpoint(c, a);
            subdivided.insert(subdivided.end(), {a, ab, ca, b, bc, ab, c, ca, bc, ab, bc, ca});
        }
        indices = std::move(subdivided);
    }

    std::vector<Vertex> vertices;
    vertices.reserve(positions.size());
    for (const auto& normal : positions) {
        float theta = std::atan2(normal.z, normal.x);
        float u = 0.5f + theta / (2.0f * static_cast<float>(M_PI));
        float v = std::acos(std::clamp(normal.y, -1.0f, 1.0f)) / static_cast<float>(M_PI);
        glm::vec4 tangent = {-std::sin(theta), 0.0f, std::cos(theta), 1.0f};
        vertices.push_back({normal * radius, 0, tangent, normal, 0, {u, v}, {0, 0}});
    }

    return _createModel(vertices, indices, color);
}

std::optional<DrawableModel> PrimitivesBuilder::_createModel(
    const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, vax::engine::Color color
) {
    auto mesh = _resourceManager.get().meshManager().createEmptyMesh();
    if (!mesh)
        return std::nullopt;
    mesh.value().second->setVertices(vertices);
    for (uint32_t index : indices) {
        mesh.value().second->addIndex(index);
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