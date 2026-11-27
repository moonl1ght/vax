#pragma once

#include "editorRenderer.h"
#include "editorScene.h"
#include "inputController.h"
#include "sceneLoader.h"
#include "simulationEngine.h"
#include "threadRunner.h"
#include <functional>
#include <string>

namespace vax::editor {
class EditorController final : public app::InputController::Observer {
  public:
    EditorController(
        rhi::VulkanEngine& vkEngine,
        rhi::PipelineManager& pipelineManager,
        rhi::ResourceManager& resourceManager,
        core::ThreadRunner& mainThreadRunner,
        gfx::EditorRenderer& editorRenderer
    ) noexcept
        : _vkEngine(vkEngine)
        , _pipelineManager(pipelineManager)
        , _resourceManager(resourceManager)
        , _mainThreadRunner(mainThreadRunner)
        , _editorRenderer(editorRenderer) {}

    ~EditorController() noexcept = default;

    EditorController(const EditorController& other) = delete;
    EditorController& operator=(const EditorController& other) = delete;
    EditorController(EditorController&& other) noexcept = delete;
    EditorController& operator=(EditorController&& other) noexcept = delete;

    void onMouseMove(const app::MouseMoveValue& value);

    void onMouseWheel(float delta);

    void onKeyEvent(const app::KeyEvent& keyEvent);

    void draw(const time::FrameTime& frameTime) noexcept;

    void load(const std::string& scenePath, std::function<void(bool)> onLoaded) noexcept;

    void cancelLoading() noexcept;

  private:
    dev::Logger _logger = dev::Logger("EditorController");

    std::reference_wrapper<rhi::VulkanEngine> _vkEngine;
    std::reference_wrapper<rhi::PipelineManager> _pipelineManager;
    std::reference_wrapper<rhi::ResourceManager> _resourceManager;
    std::reference_wrapper<core::ThreadRunner> _mainThreadRunner;
    std::reference_wrapper<gfx::EditorRenderer> _editorRenderer;

    std::unique_ptr<scene::SceneLoader> _sceneLoader;

    std::unique_ptr<scene::EditorScene> _editorScene;

    std::unique_ptr<sim::SimulationEngine> _simulationEngine;
};
} // namespace vax::editor