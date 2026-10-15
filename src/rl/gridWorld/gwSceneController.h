#pragma once

#include "animationGroup.h"
#include "camera.h"
#include "frameTime.h"
#include "gwenv.h"
#include "logger.h"
#include "sceneController.h"
#include "sceneDescriptor.h"
#include "vaxMath.h"
#include "components.h"
#include <string_view>
#include <unordered_map>

namespace vax::rl {
class GWSceneController final : public vax::engine::SceneController<GWSceneController> {
  public:
    GWSceneController(std::unique_ptr<vax::engine::Scene> scene, std::shared_ptr<vax::ecs::World> world)
        : SceneController(std::move(scene), std::move(world)) {
        _scene->agentCamera() = vax::engine::Camera(
            vax::math::SizeUI(640, 480), vax::engine::Camera::Projection::perspective, glm::vec3(0.0f, 0.0f, 0.0f)
        );
        _scene->agentCamera().setAim(vax::engine::Camera::Aim::free);
    }

    ~GWSceneController() = default;

    GWSceneController(const GWSceneController& other) = delete;
    GWSceneController& operator=(const GWSceneController& other) = delete;
    GWSceneController(GWSceneController&& other) noexcept = delete;
    GWSceneController& operator=(GWSceneController&& other) noexcept = delete;

    bool load(const vax::engine::SceneDescriptor& descriptor);

    void onUpdate(const vax::engine::SceneUpdateContext& sceneUpdateContext);

    void moveAgentTo(
        vax::math::Position2DFloat position,
        vax::rl::AgentOrientation orientation,
        bool withAnimation,
        float moveSpeed = 2.0f,
        float rotationSpeed = 1.0f
    );

    bool isMovingAgent() const;

    void resetInstancesHighlight(std::string instanceId);

    void highlightInstance(std::string instanceId, uint32_t instanceIndex, vax::engine::Color color);

    void setOnAllAnimationsCompleted(std::function<void()> onAllAnimationsCompleted);

  private:
    struct AgentState final {
        std::optional<vax::math::Position2DFloat> position;
        std::optional<int> orientation;
        std::optional<float> latestRotationEnd;
    };

    struct Wheel final {
        vax::ecs::Entity entity;
        vax::math::Transform jointOrigin;
    };

    vax::Logger _logger = vax::Logger("GWDrawableWorld");

    std::unordered_map<std::string, std::vector<vax::ecs::Entity>> _envEntities;

    vax::ecs::Entity _agent = vax::ecs::NullEntity;
    std::vector<Wheel> _wheels;
    AgentState _agentState;

    std::optional<vax::AnimationGroup> _animations;
    std::optional<std::function<void()>> _onAllAnimationsCompleted;

    bool _loadAgent();

    void _spinWheels(const vax::engine::FrameTime& frameTime);

    vax::ecs::Entity _findInSubtree(vax::ecs::Entity root, std::string_view name);

    template <typename F> void _updateAgentTransform(F&& update) {
        auto local = _world->getCoreComponentFor<vax::ecs::LocalTransformComponent>(_agent);
        if (!local) {
            return;
        }
        update(local->transform);
        _world->addCoreComponentFor<vax::ecs::TransformDirtyComponent>(_agent);
    }

    template <typename F> void _forEachInSubtree(vax::ecs::Entity root, F&& visit) {
        std::vector<vax::ecs::Entity> stack = {root};
        while (!stack.empty()) {
            vax::ecs::Entity entity = stack.back();
            stack.pop_back();
            if (!_world->isEntityAlive(entity)) {
                continue;
            }
            visit(entity);
            if (!_world->hasCoreComponent<vax::ecs::HierarchyComponent>(entity)) {
                continue;
            }
            vax::ecs::Entity child = _world->getCoreComponentFor<vax::ecs::HierarchyComponent>(entity)->firstChild;
            while (!child.isNull() && _world->hasCoreComponent<vax::ecs::HierarchyComponent>(child)) {
                stack.push_back(child);
                child = _world->getCoreComponentFor<vax::ecs::HierarchyComponent>(child)->nextSibling;
            }
        }
    }
};
} // namespace vax::rl