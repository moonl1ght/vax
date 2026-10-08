#include "gridWorldSceneBuilder.h"
#include "entityDescriptor.h"

using namespace vax::rl;
using namespace vax;
using namespace vax::engine;
using namespace vax::math;

vax::engine::SceneDescriptor GridWorldSceneBuilder::buildScene(
    const vax::math::Tensor& grid,
    std::vector<vax::math::Position2DFloat> drawableWorldPositions,
    vax::rl::GWAgent& agent
) const {
    SceneDescriptor sceneDescriptor;
    sceneDescriptor.name = "GridWorld";

    std::unordered_map<std::string, engine::EntityDescriptor> descriptors;

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
            auto instanceColor = engine::ColorPalette::Clear;
            bool isSelected = false;
            if (blockType == GridWorld::BlockType::START) {
                instanceColor = engine::ColorPalette::Blue;
                isSelected = true;
            }
            if (blockType == GridWorld::BlockType::FINISH) {
                instanceColor = engine::ColorPalette::Green;
                isSelected = true;
            }
            auto assetDescriptor = engine::PrefabDescriptor::AssetDescriptor{
                .path = blockTypeString,
            };
            auto instanceInfo = engine::PrefabDescriptor::InstanceInfo{isSelected, transform, instanceColor};
            auto prefabDescriptor = engine::PrefabDescriptor{
                .prefabType = engine::PrefabDescriptor::PrefabType::ASSET,
                .id = blockTypeString,
                .instanceInfos = {instanceInfo},
                .assetDescriptor = assetDescriptor,
            };
            descriptors[blockTypeString] = engine::EntityDescriptor{
                .id = blockTypeString,
                .type = engine::EntityDescriptor::Type::Environment,
                .prefabDescriptor = prefabDescriptor,
            };
        } else {
            auto& descriptor = descriptors[blockTypeString];
            auto instanceColor = engine::ColorPalette::Clear;
            bool isSelected = false;
            if (blockType == GridWorld::BlockType::START) {
                instanceColor = engine::ColorPalette::Blue;
                isSelected = true;
            }
            if (blockType == GridWorld::BlockType::FINISH) {
                instanceColor = engine::ColorPalette::Green;
                isSelected = true;
            }
            descriptor.prefabDescriptor->instanceInfos.push_back(
                engine::PrefabDescriptor::InstanceInfo{isSelected, transform, instanceColor}
            );
        }
        ++flatIndex;
    }

    auto agentPrefabDescriptor = agent.getPrefabDescriptor();
    auto agentDescriptor = engine::EntityDescriptor{
        .id = agentPrefabDescriptor.id,
        .type = engine::EntityDescriptor::Type::Agent,
        .prefabDescriptor = agentPrefabDescriptor,
    };

    return sceneDescriptor;
}