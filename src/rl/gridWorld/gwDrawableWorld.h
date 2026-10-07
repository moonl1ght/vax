#pragma once

#include "animationGroup.h"
#include "assetsLibrary.h"
#include "camera.h"
#include "drawableWorld.h"
#include "frameTime.h"
#include "gridWorldDescriptor.h"
#include "gwenv.h"
#include "logger.h"
#include "vaxMath.h"
#include "сomponents.h"
#include <string_view>
#include <unordered_map>

namespace vax::rl {
class GWDrawableWorld final : public vax::engine::DrawableWorld {
  public:
    GWDrawableWorld(vax::ecs::World& world)
        : vax::engine::DrawableWorld(world) {
        _roverCamera = vax::engine::Camera(
            vax::math::SizeUI(640, 480), vax::engine::Camera::Projection::perspective, glm::vec3(0.0f, 0.0f, 0.0f)
        );
        _roverCamera.setAim(vax::engine::Camera::Aim::free);
    }

    ~GWDrawableWorld() = default;

    GWDrawableWorld(const GWDrawableWorld& other) = delete;
    GWDrawableWorld& operator=(const GWDrawableWorld& other) = delete;
    GWDrawableWorld(GWDrawableWorld&& other) noexcept = delete;
    GWDrawableWorld& operator=(GWDrawableWorld&& other) noexcept = delete;

    bool load(const vax::rl::GridWorldDrawableDescriptor& descriptor);

    void update(const vax::engine::FrameTime& frameTime);

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

    const vax::engine::Camera& roverCamera() const { return _roverCamera; }

    vax::engine::Camera& roverCamera() { return _roverCamera; }

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
    vax::engine::Camera _roverCamera;

    std::optional<vax::AnimationGroup> _animations;
    std::optional<std::function<void()>> _onAllAnimationsCompleted;

    bool _loadAgent();

    void _spinWheels(const vax::engine::FrameTime& frameTime);

    vax::ecs::Entity _findInSubtree(vax::ecs::Entity root, std::string_view name);

    template <typename F> void _updateAgentTransform(F&& update) {
        auto& world = _world.get();
        auto local = world.getComponentFor<vax::ecs::LocalTransformComponent>(_agent);
        if (!local.has_value()) {
            return;
        }
        update(local->transform);
        world.addComponentFor<vax::ecs::LocalTransformComponent>(_agent, *local);
        world.addComponentFor<vax::ecs::TransformDirtyComponent>(_agent);
    }

    template <typename F> void _forEachInSubtree(vax::ecs::Entity root, F&& visit) {
        auto& world = _world.get();
        std::vector<vax::ecs::Entity> stack = {root};
        while (!stack.empty()) {
            vax::ecs::Entity entity = stack.back();
            stack.pop_back();
            if (!world.isEntityAlive(entity)) {
                continue;
            }
            visit(entity);
            if (!world.hasComponent<vax::ecs::HierarchyComponent>(entity)) {
                continue;
            }
            vax::ecs::Entity child = world.getComponentFor<vax::ecs::HierarchyComponent>(entity)->firstChild;
            while (!child.isNull() && world.hasComponent<vax::ecs::HierarchyComponent>(child)) {
                stack.push_back(child);
                child = world.getComponentFor<vax::ecs::HierarchyComponent>(child)->nextSibling;
            }
        }
    }
};
} // namespace vax::rl