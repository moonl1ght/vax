#pragma once

#include "colorPalette.h"
#include "drawableModel.h"
#include "resourceManager.h"

namespace vax::engine {
class PrimitivesBuilder {
  public:
    PrimitivesBuilder(vax::vk::ResourceManager& resourceManager)
        : _resourceManager(resourceManager) {};

    ~PrimitivesBuilder() {};

    PrimitivesBuilder(const PrimitivesBuilder& other) = delete;
    PrimitivesBuilder(PrimitivesBuilder&& other) noexcept = delete;
    PrimitivesBuilder& operator=(const PrimitivesBuilder& other) = delete;
    PrimitivesBuilder& operator=(PrimitivesBuilder&& other) noexcept = delete;

    std::optional<vax::engine::DrawableModel>
    createCube(float width, float height, float depth, vax::engine::Color color);

    std::optional<vax::engine::DrawableModel> createPlane(float width, float height, vax::engine::Color color);

    std::optional<vax::engine::DrawableModel>
    createSphere(float radius, uint32_t segments, uint32_t rings, vax::engine::Color color);

    std::optional<vax::engine::DrawableModel>
    createIcosphere(float radius, uint32_t subdivisions, vax::engine::Color color);

  private:
    std::optional<vax::engine::DrawableModel> _createModel(
        const std::vector<vax::vk::Vertex>& vertices, const std::vector<uint32_t>& indices, vax::engine::Color color
    );

  private:
    std::reference_wrapper<vax::vk::ResourceManager> _resourceManager;
};
} // namespace vax::engine