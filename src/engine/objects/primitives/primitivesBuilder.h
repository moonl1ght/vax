#pragma once

#include "colorPalette.h"
#include "drawableModel.h"
#include "resourceManager.h"
#include "textureManager.h"

namespace vax::engine {
class PrimitivesBuilder {
  public:
    explicit PrimitivesBuilder(vax::vk::ResourceManager& resourceManager) noexcept
        : _resourceManager(resourceManager) {};

    ~PrimitivesBuilder() = default;

    PrimitivesBuilder(const PrimitivesBuilder& other) = delete;
    PrimitivesBuilder(PrimitivesBuilder&& other) noexcept = delete;
    PrimitivesBuilder& operator=(const PrimitivesBuilder& other) = delete;
    PrimitivesBuilder& operator=(PrimitivesBuilder&& other) noexcept = delete;

    std::optional<vax::engine::DrawableModel>
    createCube(float width, float height, float depth, PBRMaterial material) noexcept;

    std::optional<vax::engine::DrawableModel> createPlane(float width, float height, PBRMaterial material) noexcept;

    std::optional<vax::engine::DrawableModel>
    createSphere(float radius, uint32_t segments, uint32_t rings, PBRMaterial material) noexcept;

    std::optional<vax::engine::DrawableModel>
    createIcosphere(float radius, uint32_t subdivisions, PBRMaterial material) noexcept;

  private:
    std::optional<vax::engine::DrawableModel> _createModel(
        const std::vector<vax::vk::Vertex>& vertices, const std::vector<uint32_t>& indices, PBRMaterial material
    ) noexcept;

  private:
    std::reference_wrapper<vax::vk::ResourceManager> _resourceManager;
};
} // namespace vax::engine