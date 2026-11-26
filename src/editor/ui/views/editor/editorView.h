#pragma once

#include "menuView.h"
#include "editorController.h"
#include "loadingView.h"
#include <string>

namespace vax::editor {

class EditorView final : public MenuView {
  public:
    EditorView(
        vax::gfx::EditorRenderer_old& editorRenderer,
        vax::gfx::EditorRenderer* editorRenderer_new,
        std::unique_ptr<EditorController> editorController,
        std::string scenePath
    ) noexcept
        : MenuView(editorRenderer, editorRenderer_new)
        , _editorController(std::move(editorController))
        , _scenePath(std::move(scenePath))
        , _loadingView(std::make_unique<LoadingView>(editorRenderer, editorRenderer_new)) {}

    ~EditorView() override = default;

    EditorView(const EditorView& other) = delete;
    EditorView& operator=(const EditorView& other) = delete;
    EditorView(EditorView&& other) noexcept = delete;
    EditorView& operator=(EditorView&& other) noexcept = delete;

    void updateUI(const vax::time::FrameTime& frameTime) override;

    void viewWillShow() override;

    bool draw(const vax::time::FrameTime& frameTime) override;

  protected:
    bool _isMenuLocked() const override { return !_isLoaded; }

    bool _hasSimulationView() const override { return true; }

    bool _isCloseEnabled() const override { return !_isCancelling; }

    void _onClose() override;

  private:
    dev::Logger _logger = dev::Logger("EditorView");

    std::unique_ptr<EditorController> _editorController;
    std::string _scenePath;

    std::unique_ptr<LoadingView> _loadingView;
    bool _isLoading = false;
    bool _isLoaded = false;
    bool _isCancelling = false;
    bool _shouldClose = false;

    void _cancelLoadingAndClose();
};

} // namespace vax::editor