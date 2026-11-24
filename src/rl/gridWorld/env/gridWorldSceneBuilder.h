#pragma once

#include "gridWorld.h"
#include "gwAgent.h"
#include "paths.h"
#include "sceneConfig.h"
#include "tensor.h"
#include "vaxMath.h"

namespace vax::rl {
class GridWorldSceneBuilder {
  public:
    static std::string blockTypeToPath(GridWorld::BlockType blockType) {
        switch (blockType) {
        case GridWorld::BlockType::FLOOR:
            return RES_PATH("assets/models/floor.glb");
        case GridWorld::BlockType::WALL:
            return RES_PATH("assets/models/wall.glb");
        case GridWorld::BlockType::TRAP:
            return RES_PATH("assets/models/floor.glb");
        case GridWorld::BlockType::FINISH:
            return RES_PATH("assets/models/floor.glb");
        default:
            return RES_PATH("assets/models/floor.glb");
        }
    }

    GridWorldSceneBuilder() {};
    ~GridWorldSceneBuilder() = default;

    scene::SceneConfig buildScene(
        const vax::math::Tensor& grid,
        std::vector<vax::math::Position2DFloat> drawableWorldPosition,
        vax::rl::GWAgent& agent
    ) const;
};
} // namespace vax::rl
