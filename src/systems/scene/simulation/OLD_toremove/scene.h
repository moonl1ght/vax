#pragma once

#include "camera.h"
#include "drawContext.h"
#include "environmentMap.h"
#include "frameTime.h"
#include "frameUniformSlot.h"
#include "indirectDrawController.h"
#include "inputController.h"
#include "light.h"
#include "logger.h"
#include "renderContext.h"
#include "renderPassGraphDescriptor.h"
#include "resourceManager.h"
#include "sceneComposer.h"
#include "sceneConfig.h"
#include "shaderData.h"
#include "shaderResourceHandle.h"
#include "state_tracker.h"
#include "vkEngine.h"
#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace vax::rl {
struct GridWorldDrawableDescriptor;
} // namespace vax::rl

namespace vax::gfx {
struct SceneLoader;
}

namespace vax::gfx {
struct SceneUpdateContext {
    time::FrameTime frameTime;
};

class Scene final : public app::InputController::Observer {
  public:
    friend class SceneLoader;

    explicit Scene(
        vax::rhi::VulkanEngine& vkEngine,
        editor::WindowController& windowController,
        std::unique_ptr<vax::ecs::World> gizmoWorld,
        std::unique_ptr<vax::ecs::World> backgroundWorld,
        std::unique_ptr<vax::gfx::EnvironmentMap> environmentMap,
        std::unique_ptr<vax::gfx::SceneComposer> sceneComposer,
        vax::rhi::ResourceManager& resourceManager
    )
        : _vkEngine(vkEngine)
        , _windowController(windowController)
        , _gizmoWorld(std::move(gizmoWorld))
        , _backgroundWorld(std::move(backgroundWorld))
        , _environmentMap(std::move(environmentMap))
        , _sceneComposer(std::move(sceneComposer))
        , _resourceManager(resourceManager) {
        _indirectDrawController = std::make_unique<IndirectDrawController>(*_vkEngine.get().device);
    };

    ~Scene() {
        _releaseResources();
        if (_inputController) {
            _inputController->removeObserver(this);
        }
    };

    Scene(const Scene& other) = delete;
    Scene& operator=(const Scene& other) = delete;
    Scene(Scene&& other) noexcept = delete;
    Scene& operator=(Scene&& other) noexcept = delete;

    void resize();

    void load(const scene::SceneConfig& descriptor);

    void prepareForDraw(vax::gfx::RenderCallContext renderCallContext);

    void update(vax::gfx::SceneUpdateContext sceneUpdateContext);

    void registerResources() noexcept;

    void beginDrawing(vax::rhi::CommandBuffer& commandBuffer, uint32_t frameIndex);

    void draw(const vax::gfx::DrawContext& drawContext, FrameUniformSlot cameraSlot = FrameUniformSlot::MAIN_CAMERA);

    void drawShadow(const vax::gfx::DrawContext& drawContext);

    void drawBackground(const vax::gfx::DrawContext& drawContext);

    void drawGizmo(const vax::gfx::DrawContext& drawContext);

    void onMouseMove(const app::MouseMoveValue& value);

    void onMouseWheel(float delta);

    void onKeyEvent(const app::KeyEvent& keyEvent);

    bool shouldRebuildRenderPassGraph() const { return _renderPassGraphDescriptor.is_dirty(); }

    void setShouldDrawSecondaryWindow(bool shouldDrawSecondaryWindow) {
        _renderPassGraphDescriptor.update([&](vax::gfx::RenderPassGraphDescriptor& renderPassGraphDescriptor) {
            renderPassGraphDescriptor.withSecondaryWindow = shouldDrawSecondaryWindow;
        });
    }

    const vax::gfx::RenderPassGraphDescriptor& renderPassGraphDescriptor() {
        return _renderPassGraphDescriptor.get_and_clear_dirty();
    }

    vax::gfx::Camera& agentCamera() { return _agentCamera; }

    StructuredBufferShaderHandle instanceBufferHandle(uint32_t frameIndex) const {
        return _instanceBufferHandles[frameIndex];
    }

    void setWorld(std::shared_ptr<vax::ecs::World> world) { _world = std::move(world); }

  private:
    dev::Logger _logger = dev::Logger("DrawableScene");

    std::reference_wrapper<vax::rhi::ResourceManager> _resourceManager;
    std::reference_wrapper<vax::rhi::VulkanEngine> _vkEngine;
    std::reference_wrapper<editor::WindowController> _windowController;

    std::unique_ptr<IndirectDrawController> _indirectDrawController;
    std::unique_ptr<vax::gfx::EnvironmentMap> _environmentMap;
    std::unique_ptr<vax::gfx::SceneComposer> _sceneComposer;
    std::unique_ptr<vax::ecs::World> _gizmoWorld;
    std::unique_ptr<vax::ecs::World> _backgroundWorld;

    std::shared_ptr<vax::ecs::World> _world = nullptr;

    std::vector<vax::rhi::AnyBuffer*> _sceneUniformBuffers;
    std::vector<vax::rhi::AnyBuffer*> _lightsUniformBuffer;

    StructuredBufferShaderHandle _vertexBufferHandle;
    StructuredBufferShaderHandle _materialBufferHandle;
    StructuredBufferShaderHandle _environmentMapBufferHandle;
    std::array<StructuredBufferShaderHandle, vax::rhi::MAX_FRAMES_IN_FLIGHT> _instanceBufferHandles;
    std::vector<SampledImageShaderHandle> _textureHandles;
    bool _resourcesRegistered = false;

    void _releaseResources() noexcept;

    void _writeUniformDescriptorSets() noexcept;

    uint32_t _frameUniformStride() const noexcept;

    void _bindFrameUniformSlot(const vax::gfx::DrawContext& drawContext, FrameUniformSlot slot) const noexcept;

    void _bindIndexBuffer(const vax::gfx::DrawContext& drawContext) const noexcept;

    std::optional<scene::SceneConfig> _descriptor;

    ecs::Entity _gizmoEntity;
    vax::gfx::Camera _agentCamera;
    vax::gfx::Camera _gizmoCamera;
    vax::gfx::Camera _mainCamera;
    vax::gfx::Light _sunLight;
    UniformBufferObject _ubo;
    UniformBufferObject _sunLightUbo;
    UniformBufferObject _agentCameraUbo;

    vax::gfx::RenderCallContext _renderCallContext;
    vax::gfx::SceneUpdateContext _sceneUpdateContext;

    state_tracker<vax::gfx::RenderPassGraphDescriptor> _renderPassGraphDescriptor =
        state_tracker<vax::gfx::RenderPassGraphDescriptor>({.withGizmo = false, .withSecondaryWindow = false});
};
} // namespace vax::gfx