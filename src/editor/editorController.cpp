#include "editorController.h"

using namespace vax;
using namespace vax::editor;

void EditorController::onMouseMove(const app::MouseMoveValue& value) {}

void EditorController::onMouseWheel(float delta) {}

void EditorController::onKeyEvent(const app::KeyEvent& keyEvent) {}

void EditorController::draw(const time::FrameTime& frameTime) noexcept {
    auto renderResult = _editorRenderer.get().render(_editorScene.get(), frameTime);
    if (!renderResult) [[unlikely]] {
        _logger.error("Failed to render frame!");
    }
}

void EditorController::load(const std::string& scenePath, std::function<void(bool)> onLoaded) noexcept {
    _sceneLoader = std::make_unique<scene::SceneLoader>(
        _vkEngine.get(), _pipelineManager.get(), _resourceManager.get(), _mainThreadRunner.get()
    );
    _sceneLoader->asyncLoadScene(
        scenePath, [this, onLoaded = std::move(onLoaded)](std::optional<scene::SceneLoader::LoadingResult> result) {
            _sceneLoader.reset();
            if (!result) {
                _logger.error("Failed to load scene");
                onLoaded(false);
                return;
            }
            _editorScene = std::move(result->editorScene);
            _editorRenderer.get().load(result->assetsLibrary, 10000, 10000);

            _simulationEngine = std::make_unique<sim::SimulationEngine>();
            auto simultationSetupResult = _simulationEngine->setup(
                sim::SimulationConfig::defaultConfig(), result->assetsLibrary, std::move(result->simulationScene)
            );
            if (!simultationSetupResult) {
                _logger.error("Failed to setup simulation engine");
                onLoaded(false);
                return;
            }
            onLoaded(true);
        }
    );
}

void EditorController::cancelLoading() noexcept {
    if (_sceneLoader) {
        _sceneLoader->cancel();
    }
}
