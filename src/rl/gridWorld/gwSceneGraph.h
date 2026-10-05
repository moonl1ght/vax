#pragma once

#include "animationGroup.h"
#include "camera.h"
#include "drawableNode.h"
#include "frameTime.h"
#include "gridWorldDescriptor.h"
#include "gwAgentNode.h"
#include "gwenv.h"
#include "indirectDrawController.h"
#include "logger.h"
#include "modelLoader.h"
#include "assetsLibrary.h"
#include "vaxMath.h"

namespace vax::rl {
// TODO: move to generic scene graph
class GwSceneGraph final {
  public:
    GwSceneGraph() {};

    ~GwSceneGraph() = default;

    GwSceneGraph(const GwSceneGraph& other) = delete;
    GwSceneGraph& operator=(const GwSceneGraph& other) = delete;
    GwSceneGraph(GwSceneGraph&& other) noexcept = delete;
    GwSceneGraph& operator=(GwSceneGraph&& other) noexcept = delete;

    bool load(vax::engine::ModelsController& modelsController, const vax::rl::GridWorldDrawableDescriptor& descriptor);

    void prepareDrawing(engine::IndirectDrawController* indirectDrawController, uint32_t frameIndex);

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

    const vax::engine::Camera& roverCamera() const { return _gwAgentNode->camera(); }

    vax::engine::Camera& roverCamera() { return _gwAgentNode->camera(); }

  private:
    vax::Logger _logger = vax::Logger("GwSceneGraph");
    std::vector<vax::engine::DrawableNode> _envNodes;
    std::unique_ptr<vax::rl::GWAgentNode> _gwAgentNode;
    std::optional<vax::AnimationGroup> _animations;
    std::optional<std::function<void()>> _onAllAnimationsCompleted;
};
} // namespace vax::rl