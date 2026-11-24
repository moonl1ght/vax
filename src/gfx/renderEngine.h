#pragma once

#include "editor.h"
#include "editorRenderer.h"
#include "editorRenderer_old.h"
#include "logger.h"
#include "resourceManager.h"
#include "simulationRenderer.h"
#include "vkEngine.h"

namespace vax::gfx {

class RenderEngine final {
  public:
    explicit RenderEngine() noexcept {};

    ~RenderEngine() = default;

    RenderEngine(const RenderEngine& other) = delete;
    RenderEngine& operator=(const RenderEngine& other) = delete;
    RenderEngine(RenderEngine&& other) noexcept = delete;
    RenderEngine& operator=(RenderEngine&& other) noexcept = delete;

    bool setup(editor::Editor* editor) noexcept;

    void waitIdle() noexcept;

    void cleanup() noexcept;

    rhi::VulkanEngine& vkEngine() noexcept { return *_vkEngine; }

    rhi::ResourceManager& resourceManager() noexcept { return *_resourceManager; }

    gfx::EditorRenderer_old& editorRenderer_old() noexcept { return *_editorRenderer_old; }

    gfx::EditorRenderer& editorRenderer() noexcept { return *_editorRenderer; }

    gfx::SimulationRenderer& simulationRenderer() noexcept { return *_simulationRenderer; }

  private:
    dev::Logger _logger = dev::Logger("RenderEngine");

    std::unique_ptr<rhi::VulkanEngine> _vkEngine;
    std::unique_ptr<gfx::EditorRenderer> _editorRenderer;
    std::unique_ptr<gfx::EditorRenderer_old> _editorRenderer_old;
    std::unique_ptr<gfx::SimulationRenderer> _simulationRenderer;
    std::unique_ptr<rhi::ResourceManager> _resourceManager;
};

} // namespace vax::gfx