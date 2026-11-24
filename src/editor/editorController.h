#pragma once

#include "editorScene.h"
#include "inputController.h"
#include "simulationEngine.h"

namespace vax::editor {
class EditorController final : public app::InputController::Observer {
  public:
    explicit EditorController() noexcept = default;

    ~EditorController() noexcept = default;

    EditorController(const EditorController& other) = delete;
    EditorController& operator=(const EditorController& other) = delete;
    EditorController(EditorController&& other) noexcept = delete;
    EditorController& operator=(EditorController&& other) noexcept = delete;

    void onMouseMove(const app::MouseMoveValue& value);

    void onMouseWheel(float delta);

    void onKeyEvent(const app::KeyEvent& keyEvent);

    void draw();

  private:
    std::unique_ptr<scene::EditorScene> _editorScene;

    std::unique_ptr<sim::SimulationEngine> _simulationEngine;
};
} // namespace vax::editor