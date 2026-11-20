#pragma once

#include "drawableModel.h"
#include "resourceManager.h"

namespace vax::gfx {
class PrimitivesBuilder {
  public:
    explicit PrimitivesBuilder(vax::rhi::ResourceManager& resourceManager) noexcept
        : _resourceManager(resourceManager) {};

    ~PrimitivesBuilder() = default;

    PrimitivesBuilder(const PrimitivesBuilder& other) = delete;
    PrimitivesBuilder(PrimitivesBuilder&& other) noexcept = delete;
    PrimitivesBuilder& operator=(const PrimitivesBuilder& other) = delete;
    PrimitivesBuilder& operator=(PrimitivesBuilder&& other) noexcept = delete;

    std::optional<vax::gfx::DrawableModel>
    createCube(float width, float height, float depth, PBRMaterial material) noexcept;

    std::optional<vax::gfx::DrawableModel> createPlane(float width, float height, PBRMaterial material) noexcept;

    std::optional<vax::gfx::DrawableModel>
    createSphere(float radius, uint32_t segments, uint32_t rings, PBRMaterial material) noexcept;

    std::optional<vax::gfx::DrawableModel>
    createIcosphere(float radius, uint32_t subdivisions, PBRMaterial material) noexcept;

  private:
    std::optional<vax::gfx::DrawableModel> _createModel(
        const std::vector<vax::rhi::Vertex>& vertices, const std::vector<uint32_t>& indices, PBRMaterial material
    ) noexcept;

  private:
    std::reference_wrapper<vax::rhi::ResourceManager> _resourceManager;
};
} // namespace vax::gfx