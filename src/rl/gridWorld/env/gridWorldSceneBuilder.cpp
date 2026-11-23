#include "gridWorldSceneBuilder.h"
#include "entityDescriptor.h"

using namespace vax::rl;
using namespace vax;
using namespace vax::gfx;
using namespace vax::math;

vax::gfx::SceneDescriptor GridWorldSceneBuilder::buildScene(
    const vax::math::Tensor& grid,
    std::vector<vax::math::Position2DFloat> drawableWorldPositions,
    vax::rl::GWAgent& agent
) const {
    SceneDescriptor sceneDescriptor;
    sceneDescriptor.name = "GridWorld";
    sceneDescriptor.shouldDrawGizmo = true;
    sceneDescriptor.sceneType = SceneDescriptor::SceneType::GridWorld;

    std::unordered_map<std::string, gfx::EntityDescriptor> descriptors;

    int flatIndex = 0;
    for (const auto& block : grid) {
        GridWorld::BlockType blockType = static_cast<GridWorld::BlockType>(block);
        auto blockTypeString = blockTypeToPath(blockType);
        Transform transform = Transform();
        transform.position = {drawableWorldPositions[flatIndex].x, 0.0f, drawableWorldPositions[flatIndex].y};
        if (blockType == GridWorld::BlockType::WALL) {
            transform.position.y = 0.5f;
        }

        if (descriptors.find(blockTypeString) == descriptors.end()) {
            auto instanceColor = gfx::ColorPalette::Clear;
            bool isSelected = false;
            if (blockType == GridWorld::BlockType::START) {
                instanceColor = gfx::ColorPalette::Blue;
                isSelected = true;
            }
            if (blockType == GridWorld::BlockType::FINISH) {
                instanceColor = gfx::ColorPalette::Green;
                isSelected = true;
            }
            auto assetDescriptor = gfx::PrefabDescriptor::AssetDescriptor{
                .path = blockTypeString,
            };
            auto instanceInfo = gfx::PrefabDescriptor::InstanceInfo{isSelected, transform, instanceColor};
            auto prefabDescriptor = gfx::PrefabDescriptor{
                .prefabType = gfx::PrefabDescriptor::PrefabType::ASSET,
                .id = blockTypeString,
                .instanceInfos = {instanceInfo},
                .assetDescriptor = assetDescriptor,
            };
            descriptors[blockTypeString] = gfx::EntityDescriptor{
                .id = blockTypeString,
                .type = gfx::EntityDescriptor::Type::Environment,
                .prefabDescriptor = prefabDescriptor,
            };
        } else {
            auto& descriptor = descriptors[blockTypeString];
            auto instanceColor = gfx::ColorPalette::Clear;
            bool isSelected = false;
            if (blockType == GridWorld::BlockType::START) {
                instanceColor = gfx::ColorPalette::Blue;
                isSelected = true;
            }
            if (blockType == GridWorld::BlockType::FINISH) {
                instanceColor = gfx::ColorPalette::Green;
                isSelected = true;
            }
            descriptor.prefabDescriptor->instanceInfos.push_back(
                gfx::PrefabDescriptor::InstanceInfo{isSelected, transform, instanceColor}
            );
        }
        ++flatIndex;
    }

    for (const auto& descriptor : descriptors) {
        sceneDescriptor.entities.push_back(descriptor.second);
    }

    auto agentPrefabDescriptor = agent.getPrefabDescriptor();
    auto agentDescriptor = gfx::EntityDescriptor{
        .id = agentPrefabDescriptor.id,
        .type = gfx::EntityDescriptor::Type::Agent,
        .prefabDescriptor = agentPrefabDescriptor,
    };
    sceneDescriptor.entities.push_back(agentDescriptor);

    return sceneDescriptor;
}