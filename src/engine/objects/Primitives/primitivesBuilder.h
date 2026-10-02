#pragma once

#include "colorPalette.h"
#include "drawableModel.h"
#include "resourceManager.h"

namespace vax::vk {
class CommandManager;
class QueueManager;
} // namespace vax::vk

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

    std::optional<vax::engine::DrawableModel> createCube(float size, vax::engine::Color color);

    std::optional<vax::engine::DrawableModel> createPlane();

  private:
    std::reference_wrapper<vax::vk::ResourceManager> _resourceManager;
    // std::reference_wrapper<vax::vk::CommandManager> _commandManager;
    // std::reference_wrapper<vax::vk::QueueManager> _queueManager;
};
} // namespace vax::engine