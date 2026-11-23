#pragma once

#include "inputController.h"
#include "mainSceneController.h"
#include "sceneView.h"
#include "vkEngine.h"

namespace vax::editor {
class PhysicsDemoView final : public SceneView<app::MainSceneController> {
  public:
    PhysicsDemoView(vax::gfx::RenderEngine& renderEngine, app::InputController& inputController)
        : SceneView(renderEngine, inputController) {}

    ~PhysicsDemoView() = default;

    PhysicsDemoView(const PhysicsDemoView& other) = delete;
    PhysicsDemoView& operator=(const PhysicsDemoView& other) = delete;
    PhysicsDemoView(PhysicsDemoView&& other) noexcept = delete;
    PhysicsDemoView& operator=(PhysicsDemoView&& other) noexcept = delete;

    void updateUI(const vax::gfx::FrameTime& frameTime) override;

    app::AppMode getAppMode() override { return app::AppMode::Continous; }

  protected:
    bool _load() override;

    const char* _loadingMessage() const override { return "Loading physics demo"; }
};
} // namespace vax::editor