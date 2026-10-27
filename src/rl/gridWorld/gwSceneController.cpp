#include "gwSceneController.h"
#include "colorPalette.h"
#include "entityDescriptor.h"
#include "shaderSharedUtils.h"

using namespace vax;
using namespace vax::math;
using namespace vax::rl;
using namespace vax::engine;

namespace {
constexpr std::array<std::string_view, 4> WheelLinkNames = {
    "front_right_wheel_link",
    "front_left_wheel_link",
    "rear_right_wheel_link",
    "rear_left_wheel_link",
};
} // namespace

bool GWSceneController::load(const vax::engine::SceneDescriptor& descriptor) {
    if (!_loadAgent()) {
        return false;
    }

    _envEntities.clear();

    _world->eachCoreComponentEntity<vax::ecs::InstanceComponent>([&](vax::ecs::Entity entity,
                                                               vax::ecs::InstanceComponent& instance) {
        if (instance.typeIndex >= descriptor.entities.size()) {
            return;
        }
        auto& entities = _envEntities[descriptor.entities[instance.typeIndex].id];
        if (entities.size() <= instance.instanceIndex) {
            entities.resize(instance.instanceIndex + 1, ecs::NullEntity);
        }
        entities[instance.instanceIndex] = entity;
    });

    auto envEntities = descriptor.forEachEntityOfType(engine::EntityDescriptor::Type::Environment);
    for (const auto& envEntity : envEntities) {
        if (!_envEntities.contains(envEntity.id)) {
            _logger.error("No entities spawned for model: ", envEntity.id);
            continue;
        }
        for (size_t instanceIndex = 0; instanceIndex < envEntity.prefabDescriptor->instanceInfos.size();
             ++instanceIndex) {
            const auto& instanceInfo = envEntity.prefabDescriptor->instanceInfos[instanceIndex];
            if (instanceInfo.isSelected) {
                highlightInstance(envEntity.id, instanceIndex, instanceInfo.selectionColor);
            }
        }
    }

    return true;
}

bool GWSceneController::_loadAgent() {
    _agent = vax::ecs::NullEntity;
    _world->eachCoreComponentEntity<vax::ecs::AgentComponent>([&](vax::ecs::Entity entity, vax::ecs::AgentComponent&) {
        _agent = entity;
    });
    if (_agent.isNull()) {
        _logger.error("Agent entity not spawned");
        return false;
    }

    std::vector<ecs::Entity> agentEntities;
    _forEachInSubtree(_agent, [&](ecs::Entity entity) { agentEntities.push_back(entity); });
    for (ecs::Entity entity : agentEntities) {
        _world->addCoreComponentFor<vax::ecs::HighlightComponent>(entity, packRGBA(ColorPalette::Clear));
    }

    _wheels.clear();
    for (std::string_view wheelName : WheelLinkNames) {
        ecs::Entity wheel = _findInSubtree(_agent, wheelName);
        if (wheel.isNull()) {
            _logger.warning("Wheel link not found: ", wheelName);
            continue;
        }
        auto transform = _world->getCoreComponentFor<vax::ecs::TransformComponent>(wheel);
        _wheels.push_back({.entity = wheel, .jointOrigin = transform ? transform->transform : Transform()});
    }

    _agentState = AgentState();
    _updateAgentTransform([](Transform& transform) { transform.setOrientationInDegrees({-90.0f, 0.0f, 0.0f}); });
    return true;
}

ecs::Entity GWSceneController::_findInSubtree(ecs::Entity root, std::string_view name) {
    ecs::Entity found = ecs::NullEntity;
    _forEachInSubtree(root, [&](ecs::Entity entity) {
        if (!found.isNull() || !_world->hasCoreComponent<vax::ecs::NameComponent>(entity)) {
            return;
        }
        if (_world->getCoreComponentFor<vax::ecs::NameComponent>(entity)->value == name) {
            found = entity;
        }
    });
    return found;
}

void GWSceneController::_spinWheels(const engine::FrameTime& frameTime) {
    Transform spin;
    auto timestamp = std::chrono::duration<float, std::chrono::seconds::period>(frameTime.timestamp).count();
    spin.setOrientationInDegrees({0.0f, timestamp * -100.0f, 0.0f});
    for (const auto& wheel : _wheels) {
        ecs::TransformComponent transform{
            .transform = Transform(wheel.jointOrigin.getModelMatrix() * spin.getModelMatrix())
        };
        _world->addCoreComponentFor<vax::ecs::TransformComponent>(wheel.entity, transform);
        _world->addCoreComponentFor<vax::ecs::TransformDirtyComponent>(wheel.entity);
    }
}

void GWSceneController::onUpdate(const vax::engine::SceneUpdateContext& sceneUpdateContext) {
    if (_animations.has_value()) {
        auto isCompleted = _animations->update(sceneUpdateContext.frameTime);
        _spinWheels(sceneUpdateContext.frameTime);
        if (isCompleted) {
            _animations = std::nullopt;
            if (_onAllAnimationsCompleted.has_value()) {
                _onAllAnimationsCompleted.value()();
                _onAllAnimationsCompleted = std::nullopt;
            }
        }
    }
}

bool GWSceneController::isMovingAgent() const { return _animations.has_value(); }

void GWSceneController::moveAgentTo(
    Position2DFloat position, AgentOrientation orientation, bool withAnimation, float moveSpeed, float rotationSpeed
) {
    if (_agent.isNull()) {
        _logger.warning("Agent not loaded!");
        return;
    }
    if (withAnimation) {
        auto transform = _world->getCoreComponentFor<vax::ecs::TransformComponent>(_agent);
        auto startRotation = transform ? transform->transform.getEulerAnglesInDegrees().y : 0.0f;
        if (!_animations.has_value()) {
            _animations = std::make_optional(vax::AnimationGroup(vax::AnimationGroup::Mode::SERIAL));
        } else if (_agentState.latestRotationEnd.has_value()) {
            startRotation = *_agentState.latestRotationEnd;
        }
        auto previousPosition = _agentState.position.value_or(position);
        auto orientationValue = static_cast<int>(orientation);
        auto previousOrientation = _agentState.orientation.value_or(orientationValue);
        auto orientationDelta = orientationValue - previousOrientation;
        if (orientationDelta != 0) {
            orientationDelta = orientationDelta == 3 ? -1 : orientationDelta == -3 ? 1 : orientationDelta;
            float rotation = startRotation + orientationDelta * 90.0f;
            _agentState.latestRotationEnd = rotation;
            auto animation = vax::ValueAnimation(rotationSpeed, startRotation, rotation);
            animation.addAnimationHandler([this](float value) {
                auto xValue = -cos(value * M_PI / 180.0f);
                auto zValue = sin(value * M_PI / 180.0f);
                _scene->agentCamera().setDirection({xValue, 0.0f, zValue});
                _updateAgentTransform([value](Transform& transform) {
                    transform.setOrientationInDegrees({-90.0f, value, 0.0f});
                });
            });
            _animations->pushAnimation(std::move(animation));
        }
        float startPosition;
        float endPosition;
        bool isX = false;
        if (previousPosition.x != position.x) {
            isX = true;
            startPosition = previousPosition.x;
            endPosition = position.x;
        } else {
            startPosition = previousPosition.y;
            endPosition = position.y;
        }
        auto moveAnimation = vax::ValueAnimation(moveSpeed, startPosition, endPosition);
        moveAnimation.addAnimationHandler([=, this](float value) {
            if (isX) {
                _scene->agentCamera().setPosition({value, 0.5f, position.y});
            } else {
                _scene->agentCamera().setPosition({position.x, 0.5f, value});
            }
            _updateAgentTransform([=](Transform& transform) {
                if (isX) {
                    transform.position = {value, 0.0f, position.y};
                } else {
                    transform.position = {position.x, 0.0f, value};
                }
            });
        });
        _animations->pushAnimation(std::move(moveAnimation));
    } else {
        float rotation = 0.0f;
        glm::vec3 direction = {0.0f, 0.0f, 0.0f};
        switch (orientation) {
        case AgentOrientation::NORTH:
            rotation = 90.0f;
            direction = {0.0f, 0.0f, 1.0f};
            break;
        case AgentOrientation::SOUTH:
            rotation = 270.0f;
            direction = {0.0f, 0.0f, -1.0f};
            break;
        case AgentOrientation::EAST:
            rotation = 0.0f;
            direction = {-1.0f, 0.0f, 0.0f};
            break;
        case AgentOrientation::WEST:
            rotation = 180.0f;
            direction = {1.0f, 0.0f, 0.0f};
            break;
        }
        _scene->agentCamera().setDirection(direction);
        _scene->agentCamera().setPosition({position.x, 0.5f, position.y});
        _updateAgentTransform([&](Transform& transform) {
            transform.position = {position.x, 0.0f, position.y};
            transform.setOrientationInDegrees({-90.0f, rotation, 0.0f});
        });
    }
    _agentState.orientation = static_cast<int>(orientation);
    _agentState.position = position;
}

void GWSceneController::resetInstancesHighlight(std::string instanceId) {
    auto it = _envEntities.find(instanceId);
    if (it == _envEntities.end()) {
        return;
    }
    for (ecs::Entity entity : it->second) {
        _world->removeCoreComponentFor<vax::ecs::HighlightComponent>(entity);
    }
}

void GWSceneController::highlightInstance(std::string instanceId, uint32_t instanceIndex, vax::engine::Color color) {
    auto it = _envEntities.find(instanceId);
    if (it == _envEntities.end() || instanceIndex >= it->second.size() || it->second[instanceIndex].isNull()) {
        _logger.warning("No env instance ", instanceId, "[", instanceIndex, "]");
        return;
    }
    _world->addCoreComponentFor<vax::ecs::HighlightComponent>(it->second[instanceIndex], packRGBA(color));
}

void GWSceneController::setOnAllAnimationsCompleted(std::function<void()> onAllAnimationsCompleted) {
    _onAllAnimationsCompleted = onAllAnimationsCompleted;
}