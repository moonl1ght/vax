#pragma once

#include "debugView.h"
#include "editorController.h"

namespace vax::editor {

class EditorView final : public DebugView {
  public:
    explicit EditorView(vax::gfx::EditorRenderer_old& editorRenderer) noexcept
        : DebugView(editorRenderer) {};

    ~EditorView() override = default;

    EditorView(const EditorView& other) = delete;
    EditorView& operator=(const EditorView& other) = delete;
    EditorView(EditorView&& other) noexcept = delete;
    EditorView& operator=(EditorView&& other) noexcept = delete;

  private:
    std::unique_ptr<EditorController> _editorController;
};

} // namespace vax::editor