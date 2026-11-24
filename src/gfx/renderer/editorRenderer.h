#pragma once

#include "baseRenderer.h"
#include "editorScene.h"
#include "frameTime.h"
#include "windowController.h"

namespace vax::gfx {

class EditorRenderer final : public gfx::BaseRenderer {
  public:
    explicit EditorRenderer(rhi::VulkanEngine& vkEngine, editor::WindowController& windowController)
        : BaseRenderer(vkEngine)
        , _windowController(windowController) {};

    ~EditorRenderer() override = default;

    EditorRenderer(const EditorRenderer&) = delete;
    EditorRenderer& operator=(const EditorRenderer&) = delete;
    EditorRenderer(EditorRenderer&&) = delete;
    EditorRenderer& operator=(EditorRenderer&&) = delete;

    bool render(scene::EditorScene* scene, const time::FrameTime& frameTime);

  private:
    dev::Logger _logger = dev::Logger("EditorRenderer");

    std::reference_wrapper<editor::WindowController> _windowController;
};

} // namespace vax::gfx