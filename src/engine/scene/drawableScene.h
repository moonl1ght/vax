#pragma once

#include "camera.h"
#include "descriptorSetWriter.h"
#include "drawContext.h"
#include "drawableWorld.h"
#include "environmentMap.h"
#include "frameTime.h"
#include "indirectDrawController.h"
#include "inputController.h"
#include "light.h"
#include "luna.h"
#include "renderContext.h"
#include "renderPassGraphDescriptor.h"
#include "resourceManager.h"
#include "sceneComposer.h"
#include "sceneDescriptor.h"
#include "shaderUniforms.h"
#include "state_tracker.h"
#include "vkEngine.h"

namespace vax::rl {
struct GridWorldDrawableDescriptor;
} // namespace vax::rl

namespace vax::engine {
struct SceneLoader;
} // namespace vax::engine

namespace vax::engine {
struct SceneUpdateContext {
    FrameTime frameTime;
};

class DrawableScene final : public vax::InputController::Observer {
  public:
    friend class SceneLoader;

    explicit DrawableScene(
        vax::vk::Engine& vkEngine,
        std::unique_ptr<vax::ecs::World> gizmoWorld,
        std::unique_ptr<vax::ecs::World> backgroundWorld,
        std::unique_ptr<vax::engine::DrawableWorld> drawableWorld,
        std::unique_ptr<vax::vk::ResourceManager> resourceManager,
        std::unique_ptr<vax::engine::EnvironmentMap> environmentMap,
        std::unique_ptr<vax::engine::SceneComposer> sceneComposer
    )
        : _vkEngine(vkEngine)
        , _gizmoWorld(std::move(gizmoWorld))
        , _backgroundWorld(std::move(backgroundWorld))
        , _drawableWorld(std::move(drawableWorld))
        , _resourceManager(std::move(resourceManager))
        , _environmentMap(std::move(environmentMap))
        , _sceneComposer(std::move(sceneComposer)) {
        _indirectDrawController = std::make_unique<IndirectDrawController>(*_vkEngine.get().device);
    };

    ~DrawableScene() {
        if (_inputController) {
            _inputController->removeObserver(this);
        }
    };

    DrawableScene(const DrawableScene& other) = delete;
    DrawableScene& operator=(const DrawableScene& other) = delete;
    DrawableScene(DrawableScene&& other) noexcept = delete;
    DrawableScene& operator=(DrawableScene&& other) noexcept = delete;

    void resize();

    void load(const vax::engine::SceneDescriptor& descriptor);

    void prepareForDraw(vax::engine::RenderCallContext renderCallContext);

    void update(vax::engine::SceneUpdateContext sceneUpdateContext);

    bool writeGlobalDescriptorSet(vax::vk::DescriptorSetWriter& descriptorWriter);

    bool writeFrameDescriptorSet(
        vax::vk::DescriptorSetWriter& descriptorWriter, vax::vk::DescriptorSetWriter& roverCameraDescriptorWriter
    );

    bool writePerDrawDescriptorSet(vax::vk::DescriptorSetWriter& descriptorWriter);

    void beginDrawing(vax::vk::CommandBuffer& commandBuffer, uint32_t frameIndex);

    void endDrawing(vax::vk::CommandBuffer& commandBuffer, uint32_t frameIndex);

    void draw(const vax::engine::DrawContext& drawContext);

    void drawBackground(const vax::engine::DrawContext& drawContext);

    void drawGizmo(const vax::engine::DrawContext& drawContext);

    void onMouseMove(const vax::MouseMoveValue& value);

    void onMouseWheel(float delta);

    void onKeyEvent(const vax::KeyEvent& keyEvent);

    vax::engine::DrawableWorld* drawableWorld() const { return _drawableWorld.get(); }

    bool shouldRebuildRenderPassGraph() const { return _renderPassGraphDescriptor.is_dirty(); }

    void setShouldDrawSecondaryWindow(bool shouldDrawSecondaryWindow) {
        _renderPassGraphDescriptor.update([&](vax::engine::RenderPassGraphDescriptor& renderPassGraphDescriptor) {
            renderPassGraphDescriptor.withSecondaryWindow = shouldDrawSecondaryWindow;
        });
    }

    const vax::engine::RenderPassGraphDescriptor& renderPassGraphDescriptor() {
        return _renderPassGraphDescriptor.get_and_clear_dirty();
    }

  private:
    vax::Logger _logger = vax::Logger("DrawableScene");

    std::reference_wrapper<vax::vk::Engine> _vkEngine;

    std::unique_ptr<IndirectDrawController> _indirectDrawController;
    std::unique_ptr<vax::engine::DrawableWorld> _drawableWorld;
    std::unique_ptr<vax::vk::ResourceManager> _resourceManager;
    std::unique_ptr<vax::engine::EnvironmentMap> _environmentMap;
    std::unique_ptr<vax::engine::SceneComposer> _sceneComposer;
    std::unique_ptr<vax::ecs::World> _gizmoWorld;
    std::unique_ptr<vax::ecs::World> _backgroundWorld;

    std::vector<vax::vk::AnyBuffer*> _sceneUniformBuffers;
    std::vector<vax::vk::AnyBuffer*> _roverCameraUniformBuffers;
    std::vector<vax::vk::AnyBuffer*> _lightsUniformBuffer;

    std::optional<vax::engine::SceneDescriptor> _descriptor;

    ecs::Entity _gizmoEntity;
    vax::engine::Camera _gizmoCamera;
    vax::engine::Camera _mainCamera;
    vax::engine::Light _sunLight;
    UniformBufferObject _ubo;
    UniformBufferObject _sunLightUbo;
    UniformBufferObject _roverCameraUbo;

    vax::engine::RenderCallContext _renderCallContext;
    vax::engine::SceneUpdateContext _sceneUpdateContext;

    state_tracker<vax::engine::RenderPassGraphDescriptor> _renderPassGraphDescriptor =
        state_tracker<vax::engine::RenderPassGraphDescriptor>({.withGizmo = false, .withSecondaryWindow = false});
};
} // namespace vax::engine