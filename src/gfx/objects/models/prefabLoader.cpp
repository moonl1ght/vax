#include "prefabLoader.h"
#include "drawableModel.h"
#include "resourceUtils.h"
#include "textureGenerator.h"
#include <urdf_parser/urdf_parser.h>
#include <uuid_v4.h>

using namespace vax;
using namespace vax::gfx;
using namespace vax::rhi;

void _processURDFLink(
    ModelLoader& modelsLoader,
    ResourceManager& resourceManager,
    Prefab& prefab,
    std::vector<DrawableModel>& models,
    int32_t parentIndex,
    std::string_view mainPath,
    urdf::LinkConstSharedPtr link
) {
    vax::math::Transform localTransform;
    if (link->parent_joint) {
        const auto& pose = link->parent_joint->parent_to_joint_origin_transform;
        localTransform.position = {pose.position.x, pose.position.y, pose.position.z};
        double x, y, z, w;
        pose.rotation.getQuaternion(x, y, z, w);
        localTransform.setOrientation(
            {static_cast<float>(w), static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)}
        );
    }

    auto linkIndex = static_cast<int32_t>(prefab.nodes.size());
    prefab.nodes.push_back({.name = link->name, .localTransform = localTransform, .parentIndex = parentIndex});

    for (size_t i = 0; i < link->visual_array.size(); ++i) {
        const auto& visual = link->visual_array[i];
        if (!visual || !visual->geometry || visual->geometry->type != urdf::Geometry::MESH) {
            continue;
        }
        MaterialId materialId = NO_MATERIAL_INDEX;
        if (visual->material) {
            PBRMaterial material{
                .baseColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
                .metallicFactor = 1.0f,
                .roughnessFactor = 1.0f,
                .normalScale = 1.0f,
                .occlusionStrength = 1.0f,
                .emissiveFactorAlphaCutoff = glm::vec4(0.0f, 0.0f, 0.0f, 0.5f),
            };
            material.baseColor =
                glm::vec4(visual->material->color.r, visual->material->color.g, visual->material->color.b, 1.0f);
            materialId = resourceManager.materialManager().insert(material);
        }
        const auto* mesh = static_cast<const urdf::Mesh*>(visual->geometry.get());
        auto model = modelsLoader.loadModel(std::string(mainPath) + "/" + mesh->filename);
        if (model.has_value()) {
            for (size_t submeshIndex = 0; submeshIndex < model->submeshCount(); ++submeshIndex) {
                model->submesh(submeshIndex).materialIndex = materialId;
            }
            auto modelId = models.size();
            models.push_back(std::move(*model));
            prefab.nodes.push_back({
                .name = link->name + "/visual" + std::to_string(i),
                .model = modelId,
                .parentIndex = linkIndex,
            });
        }
    }

    for (const auto& child : link->child_links) {
        _processURDFLink(modelsLoader, resourceManager, prefab, models, linkIndex, mainPath, child);
    }
}

std::optional<std::pair<Prefab, std::vector<DrawableModel>>>
PrefabLoader::loadPrefab(const scene::PrefabConfig& config) {
    switch (config.prefabType) {
    case scene::PrefabConfig::PrefabType::PRESET:
        return _loadPrimitivePrefab(config);
    case scene::PrefabConfig::PrefabType::ASSET:
        return _loadAssetPrefab(config);
    default:
        _logger.error("Unsupported prefab type: " + std::to_string(static_cast<int>(config.prefabType)));
        return std::nullopt;
    }
}

std::optional<std::pair<Prefab, std::vector<DrawableModel>>>
PrefabLoader::_loadPrimitivePrefab(const scene::PrefabConfig& config) {
    auto primitiveConfig = config.primitiveDescriptor;
    if (!primitiveConfig) {
        _logger.error("Primitive descriptor is null");
        return std::nullopt;
    }
    std::vector<DrawableModel> models;
    PBRMaterial material;
    auto materialConfig = primitiveConfig->materialConfig;
    material.baseColor = primitiveConfig->color;
    material.baseColorTextureIndex = NO_TEXTURE_FLAG;
    material.metallicFactor = 0.0f;
    material.roughnessFactor = 1.0f;
    if (materialConfig && materialConfig->textureConfig.generatedTextureConfig) {

        auto textureGenerator = TextureGenerator(*_textureLoader);
        auto texture = textureGenerator.generateCheckerboardTexture(materialConfig->textureConfig.name, 512, 512, 64);
        if (texture) {
            _textureGpuUploader.get().addTexture(texture.get());
            material.baseColorTextureIndex = texture->sampledShaderHandle().index;
            _resourceManager.get().textureLibrary().addTexture(std::move(texture));
        }
    }
    switch (primitiveConfig->primitiveType) {
    case scene::PrefabConfig::PrimitiveType::CUBE: {
        auto cube = std::get<scene::PrefabConfig::CubePrimitiveDescriptor>(primitiveConfig->getShapeDescriptor());
        models.push_back(
            std::move(_primitivesBuilder->createCube(cube.width, cube.height, cube.depth, material).value())
        );
        break;
    }
    case scene::PrefabConfig::PrimitiveType::PLANE: {
        auto plane = std::get<scene::PrefabConfig::PlanePrimitiveDescriptor>(primitiveConfig->getShapeDescriptor());
        models.push_back(std::move(_primitivesBuilder->createPlane(plane.width, plane.height, material).value()));
        break;
    }
    case scene::PrefabConfig::PrimitiveType::SPHERE: {
        auto sphere = std::get<scene::PrefabConfig::SpherePrimitiveDescriptor>(primitiveConfig->getShapeDescriptor());
        models.push_back(
            std::move(_primitivesBuilder->createSphere(sphere.radius, sphere.segments, sphere.rings, material).value())
        );
        break;
    }
    case scene::PrefabConfig::PrimitiveType::ICOSPHERE: {
        auto icosphere =
            std::get<scene::PrefabConfig::IcospherePrimitiveDescriptor>(primitiveConfig->getShapeDescriptor());
        models.push_back(
            std::move(_primitivesBuilder->createIcosphere(icosphere.radius, icosphere.subdivisions, material).value())
        );
        break;
    }
    default:
        _logger.error(
            "Unsupported primitive type: " + std::to_string(static_cast<int>(primitiveConfig->primitiveType))
        );
        return std::nullopt;
    }
    if (models.empty()) {
        _logger.error("Failed to create primitive");
        return std::nullopt;
    }
    Prefab prefab{
        .id = config.id,
        .nodes = {{.name = config.id, .model = 0}},
    };
    return std::optional<std::pair<Prefab, std::vector<DrawableModel>>>(
        std::in_place, std::make_pair(std::move(prefab), std::move(models))
    );
}

std::optional<std::pair<Prefab, std::vector<DrawableModel>>>
PrefabLoader::_loadAssetPrefab(const scene::PrefabConfig& config) {
    switch (config.assetDescriptor->getAssetExtension()) {
    case scene::PrefabConfig::AssetExtension::GLB:
        return _loadGLBPrefab(config);
    case scene::PrefabConfig::AssetExtension::URDF:
        return _loadURDFPrefab(config);
    default:
        _logger.error(
            "Unsupported asset extension: " +
            std::to_string(static_cast<int>(config.assetDescriptor->getAssetExtension()))
        );
        return std::nullopt;
    }
}

std::optional<std::pair<Prefab, std::vector<DrawableModel>>>
PrefabLoader::_loadURDFPrefab(const scene::PrefabConfig& config) {
    if (!config.assetDescriptor) {
        _logger.error("Asset descriptor is null");
        return std::nullopt;
    }
    auto path = config.assetDescriptor->path;
    auto model = urdf::parseURDFFile(path);
    if (!model) {
        _logger.error("Failed to load URDF model: " + path);
        return std::nullopt;
    }
    auto mainPath = config.assetDescriptor->getMainPath();
    std::vector<DrawableModel> models;
    Prefab prefab;
    prefab.id = config.id;
    _processURDFLink(*_modelLoader, _resourceManager.get(), prefab, models, -1, mainPath, model->getRoot());
    return std::optional<std::pair<Prefab, std::vector<DrawableModel>>>(
        std::in_place, std::make_pair(std::move(prefab), std::move(models))
    );
}

std::optional<std::pair<Prefab, std::vector<DrawableModel>>>
PrefabLoader::_loadGLBPrefab(const scene::PrefabConfig& config) {
    if (!config.assetDescriptor) {
        _logger.error("Asset descriptor is null");
        return std::nullopt;
    }
    auto model = _modelLoader->loadModel(config.assetDescriptor->path);
    if (!model.has_value()) {
        _logger.error("Failed to load GLB model: " + config.assetDescriptor->path);
        return std::nullopt;
    }
    std::vector<DrawableModel> models;
    models.push_back(std::move(*model));
    Prefab prefab{
        .id = config.id,
        .nodes = {{.name = config.id, .model = 0}},
    };
    return std::optional<std::pair<Prefab, std::vector<DrawableModel>>>(
        std::in_place, std::make_pair(std::move(prefab), std::move(models))
    );
}
