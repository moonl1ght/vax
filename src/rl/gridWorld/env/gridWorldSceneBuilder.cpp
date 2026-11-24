#include "gridWorldSceneBuilder.h"
#include "entityConfig.h"

using namespace vax::rl;
using namespace vax;
using namespace vax::gfx;
using namespace vax::math;

scene::SceneConfig GridWorldSceneBuilder::buildScene(
    const vax::math::Tensor& grid,
    std::vector<vax::math::Position2DFloat> drawableWorldPositions,
    vax::rl::GWAgent& agent
) const {
    scene::SceneConfig sceneDescriptor;
    sceneDescriptor.name = "GridWorld";
    sceneDescriptor.shouldDrawGizmo = true;
    sceneDescriptor.sceneType = scene::SceneConfig::SceneType::GridWorld;

    std::unordered_map<std::string, scene::EntityConfig> entityConfigs;

    int flatIndex = 0;
    for (const auto& block : grid) {
        GridWorld::BlockType blockType = static_cast<GridWorld::BlockType>(block);
        auto blockTypeString = blockTypeToPath(blockType);
        Transform transform = Transform();
        transform.position = {drawableWorldPositions[flatIndex].x, 0.0f, drawableWorldPositions[flatIndex].y};
        if (blockType == GridWorld::BlockType::WALL) {
            transform.position.y = 0.5f;
        }

        if (entityConfigs.find(blockTypeString) == entityConfigs.end()) {
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
            auto assetDescriptor = scene::PrefabConfig::AssetDescriptor{
                .path = blockTypeString,
            };
            auto instanceInfo = scene::PrefabConfig::InstanceInfo{isSelected, transform, instanceColor};
            auto prefabConfig = scene::PrefabConfig{
                .prefabType = scene::PrefabConfig::PrefabType::ASSET,
                .id = blockTypeString,
                .instanceInfos = {instanceInfo},
                .assetDescriptor = assetDescriptor,
            };
            entityConfigs[blockTypeString] = scene::EntityConfig{
                .id = blockTypeString,
                .type = scene::EntityConfig::Type::Environment,
                .prefabConfig = prefabConfig,
            };
        } else {
            auto& entityConfig = entityConfigs[blockTypeString];
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
            entityConfig.prefabConfig->instanceInfos.push_back(
                scene::PrefabConfig::InstanceInfo{isSelected, transform, instanceColor}
            );
        }
        ++flatIndex;
    }

    for (const auto& entityConfig : entityConfigs) {
        sceneDescriptor.entities.push_back(entityConfig.second);
    }

    auto agentPrefabConfig = agent.getPrefabConfig();
    auto agentEntityConfig = scene::EntityConfig{
        .id = agentPrefabConfig.id,
        .type = scene::EntityConfig::Type::Agent,
        .prefabConfig = agentPrefabConfig,
    };
    sceneDescriptor.entities.push_back(agentEntityConfig);

    return sceneDescriptor;
}