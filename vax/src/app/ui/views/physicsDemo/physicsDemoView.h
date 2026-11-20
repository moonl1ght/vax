#pragma once

#include "inputController.h"
#include "mainSceneController.h"
#include "sceneView.h"
#include "vkEngine.h"

namespace vax::ui {
class PhysicsDemoView final : public SceneView<MainSceneController> {
  public:
    PhysicsDemoView(vax::rendering::RenderingEngine& renderingEngine, vax::InputController& inputController)
        : SceneView(renderingEngine, inputController) {}

    ~PhysicsDemoView() = default;

    PhysicsDemoView(const PhysicsDemoView& other) = delete;
    PhysicsDemoView& operator=(const PhysicsDemoView& other) = delete;
    PhysicsDemoView(PhysicsDemoView&& other) noexcept = delete;
    PhysicsDemoView& operator=(PhysicsDemoView&& other) noexcept = delete;

    void updateUI(const vax::engine::FrameTime& frameTime) override;

    vax::AppMode getAppMode() override { return vax::AppMode::Continous; }

  protected:
    bool _load() override;

    const char* _loadingMessage() const override { return "Loading physics demo"; }
};
} // namespace vax::ui