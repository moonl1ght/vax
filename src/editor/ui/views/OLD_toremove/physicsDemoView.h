#pragma once

#include "inputController.h"
#include "mainSceneController.h"
#include "sceneView.h"
#include "vkEngine.h"

namespace vax::gfx {
class RenderEngine;
}

namespace vax::editor {
class PhysicsDemoView final : public SceneView<app::MainSceneController> {
  public:
    PhysicsDemoView(
        vax::gfx::EditorRenderer_old& editorRenderer,
        gfx::RenderEngine& rendererEngine,
        WindowController& windowController,
        app::InputController& inputController
    )
        : SceneView(editorRenderer, inputController)
        , _rendererEngine(rendererEngine)
        , _windowController(windowController) {}

    ~PhysicsDemoView() = default;

    PhysicsDemoView(const PhysicsDemoView& other) = delete;
    PhysicsDemoView& operator=(const PhysicsDemoView& other) = delete;
    PhysicsDemoView(PhysicsDemoView&& other) noexcept = delete;
    PhysicsDemoView& operator=(PhysicsDemoView&& other) noexcept = delete;

    void updateUI(const vax::time::FrameTime& frameTime) override;

    app::AppMode getAppMode() override { return app::AppMode::Continous; }

  protected:
    std::reference_wrapper<gfx::RenderEngine> _rendererEngine;
    std::reference_wrapper<WindowController> _windowController;

    bool _load() override;

    const char* _loadingMessage() const override { return "Loading physics demo"; }
};
} // namespace vax::editor