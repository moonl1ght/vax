#include "gwSceneGraph.h"
#include "colorPalette.h"
#include "gridWorldDescriptor.h"
#include "modelDescriptor.h"

using namespace vax;
using namespace vax::math;
using namespace vax::rl;
using namespace vax::engine;

bool GwSceneGraph::load(
    engine::AssetsLibrary& modelsController, const vax::rl::GridWorldDrawableDescriptor& descriptor
) {
    auto agentNode = modelsController.getPreloadedDrawableNodeById(descriptor.agentDrawableDescriptor.id, 1);
    if (!agentNode.has_value()) {
        _logger.error("Failed to load agent model: {}", descriptor.agentDrawableDescriptor.id);
        return false;
    }
    agentNode->setIsSelected(true);
    agentNode->setNodeSelectionColor(ColorPalette::Clear);
    _gwAgentNode =
        std::make_unique<vax::rl::GWAgentNode>(std::make_unique<vax::engine::DrawableNode>(std::move(agentNode.value())));

    _gwAgentNode->agentNode().updateTransform([&](TransformHandle& transformHandle) {
        transformHandle.updateTransform([&](Transform& transform) {
            transform.updateRotationInDegrees({-90.0f, 0.0f, 0.0f});
        });
    });

    _envNodes.reserve(descriptor.drawableDescriptors.size());
    for (const auto& drawableDescriptor : descriptor.drawableDescriptors) {
        auto node = modelsController.createDrawableNodeById(drawableDescriptor.id, drawableDescriptor.transforms);
        for (auto& selectedInstanceInfo : drawableDescriptor.selectedInstanceInfos) {
            node->selectInstance(selectedInstanceInfo.instanceIndex);
            node->setSelectionColor(selectedInstanceInfo.instanceIndex, selectedInstanceInfo.color);
        }
        if (!node.has_value()) {
            _logger.error("Failed to load model: {}", drawableDescriptor.id);
            continue;
        }
        _envNodes.push_back(std::move(node.value()));
    }

    return true;
}

void GwSceneGraph::prepareDrawing(engine::IndirectDrawController* indirectDrawController, uint32_t frameIndex) {
    if (_gwAgentNode) {
        _gwAgentNode->agentNode().prepareDrawing(indirectDrawController, frameIndex);
    } else {
        _logger.warning("Agent node not loaded!");
    }
    for (auto& node : _envNodes) {
        node.prepareDrawing(indirectDrawController, frameIndex);
    }
}

void GwSceneGraph::update(const engine::FrameTime& frameTime) {
    if (_animations.has_value()) {
        auto isCompleted = _animations->update(frameTime);
        if (_gwAgentNode) {
            _gwAgentNode->update(frameTime);
        } else {
            _logger.warning("Rover model proxy not loaded!");
        }
        if (isCompleted) {
            _animations = std::nullopt;
            if (_onAllAnimationsCompleted.has_value()) {
                _onAllAnimationsCompleted.value()();
                _onAllAnimationsCompleted = std::nullopt;
            }
        }
    }
}

bool GwSceneGraph::isMovingAgent() const { return _animations.has_value(); }

void GwSceneGraph::moveAgentTo(
    Position2DFloat position, AgentOrientation orientation, bool withAnimation, float moveSpeed, float rotationSpeed
) {
    if (_gwAgentNode) {
        if (withAnimation) {
            auto startRotation = _gwAgentNode->agentNode().getTransform().getRotationInDegrees().y;
            if (!_animations.has_value()) {
                _animations = std::make_optional(vax::AnimationGroup(vax::AnimationGroup::Mode::SERIAL));
            } else {
                if (auto metadata = _gwAgentNode->agentNode().getMetadata("latest_rotation_end_value");
                    metadata.has_value() && std::holds_alternative<float>(*metadata)) {
                    startRotation = std::get<float>(*metadata);
                }
            }
            auto previousPositionX =
                std::get<float>(_gwAgentNode->agentNode().getMetadata("position.x").value_or(position.x));
            auto previousPositionY =
                std::get<float>(_gwAgentNode->agentNode().getMetadata("position.y").value_or(position.y));
            auto orientationValue = static_cast<int>(orientation);
            auto previousOrientation = orientationValue;
            if (auto metadata = _gwAgentNode->agentNode().getMetadata("orientation");
                metadata.has_value() && std::holds_alternative<int>(*metadata)) {
                previousOrientation = std::get<int>(*metadata);
            }
            auto orientationDelta = orientationValue - previousOrientation;
            if (orientationDelta != 0) {
                orientationDelta = orientationDelta == 3 ? -1 : orientationDelta == -3 ? 1 : orientationDelta;
                float rotation = startRotation + orientationDelta * 90.0f;
                _gwAgentNode->agentNode().setMetadata("latest_rotation_end_value", rotation);
                auto animation = vax::ValueAnimation(rotationSpeed, startRotation, rotation);
                animation.addAnimationHandler([&](float value) {
                    auto xValue = -cos(value * M_PI / 180.0f);
                    auto zValue = sin(value * M_PI / 180.0f);
                    _gwAgentNode->camera().setDirection({xValue, 0.0f, zValue});
                    _gwAgentNode->agentNode().updateTransform([&](TransformHandle& transformHandle) {
                        transformHandle.updateTransform([&](Transform& transform) {
                            transform.updateRotationInDegrees({-90.0f, value, 0.0f});
                        });
                    });
                });
                _animations->pushAnimation(std::move(animation));
            }
            float startPosition;
            float endPosition;
            bool isX = false;
            if (previousPositionX != position.x) {
                isX = true;
                startPosition = previousPositionX;
                endPosition = position.x;
            } else {
                startPosition = previousPositionY;
                endPosition = position.y;
            }
            auto moveAnimation = vax::ValueAnimation(moveSpeed, startPosition, endPosition);
            moveAnimation.addAnimationHandler([=, this](float value) {
                if (isX) {
                    _gwAgentNode->camera().setPosition({value, 0.5f, position.y});
                } else {
                    _gwAgentNode->camera().setPosition({position.x, 0.5f, value});
                }
                _gwAgentNode->agentNode().updateTransform([=](TransformHandle& transformHandle) {
                    transformHandle.updateTransform([=](Transform& transform) {
                        if (isX) {
                            transform.position = {value, 0.0f, position.y};
                        } else {
                            transform.position = {position.x, 0.0f, value};
                        }
                    });
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
            _gwAgentNode->camera().setDirection(direction);
            _gwAgentNode->camera().setPosition({position.x, 0.5f, position.y});
            _gwAgentNode->agentNode().updateTransform([&](TransformHandle& transformHandle) {
                transformHandle.updateTransform([&](Transform& transform) {
                    transform.position = {position.x, 0.0f, position.y};
                    transform.updateRotationInDegrees({-90.0f, rotation, 0.0f});
                });
            });
        }
        _gwAgentNode->agentNode().setMetadata("orientation", static_cast<int>(orientation));
        _gwAgentNode->agentNode().setMetadata("position.x", position.x);
        _gwAgentNode->agentNode().setMetadata("position.y", position.y);
    } else {
        _logger.warning("Agent node not loaded!");
    }
}

void GwSceneGraph::resetInstancesHighlight(std::string instanceId) {
    for (auto& node : _envNodes) {
        if (node.name() == instanceId) {
            node.unselectAllInstances();
        }
    }
}

void GwSceneGraph::highlightInstance(std::string instanceId, uint32_t instanceIndex, vax::engine::Color color) {
    for (auto& node : _envNodes) {
        if (node.name() == instanceId) {
            node.selectInstance(instanceIndex);
            node.setSelectionColor(instanceIndex, color);
        }
    }
}

void GwSceneGraph::setOnAllAnimationsCompleted(std::function<void()> onAllAnimationsCompleted) {
    _onAllAnimationsCompleted = onAllAnimationsCompleted;
}