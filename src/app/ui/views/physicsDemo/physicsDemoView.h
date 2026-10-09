#pragma once

#include "inputController.h"
#include "sceneView.h"
#include "vkEngine.h"

namespace vax::ui {
class PhysicsDemoView final : public SceneView {
  public:
    PhysicsDemoView(vax::engine::Renderer& renderer)
        : SceneView(renderer) {}

    ~PhysicsDemoView() = default;

    PhysicsDemoView(const PhysicsDemoView& other) = delete;
    PhysicsDemoView& operator=(const PhysicsDemoView& other) = delete;
    PhysicsDemoView(PhysicsDemoView&& other) noexcept = delete;
    PhysicsDemoView& operator=(PhysicsDemoView&& other) noexcept = delete;

    void update(const vax::engine::FrameTime& frameTime) override;

    void render(const vax::engine::FrameTime& frameTime) override;

    vax::AppMode getAppMode() override { return vax::AppMode::Continous; }

    void load(vax::vk::Engine& engine, vax::InputController& inputController);

  private:
};
} // namespace vax::ui