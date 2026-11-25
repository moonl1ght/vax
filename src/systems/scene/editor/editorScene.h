#pragma once

#include "gizmo.h"

namespace vax::scene {

class EditorScene final {
  public:
    explicit EditorScene(std::unique_ptr<Gizmo> gizmo) noexcept
        : _gizmo(std::move(gizmo)) {}

    ~EditorScene() = default;

    EditorScene(const EditorScene&) = delete;
    EditorScene(EditorScene&&) noexcept = delete;
    EditorScene& operator=(const EditorScene&) = delete;
    EditorScene& operator=(EditorScene&&) noexcept = delete;

  private:
    std::unique_ptr<Gizmo> _gizmo;
};

} // namespace vax::scene