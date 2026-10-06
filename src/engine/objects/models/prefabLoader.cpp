#include "prefabLoader.h"
#include "drawableModel.h"
#include "resourceUtils.h"
#include <urdf_parser/urdf_parser.h>
#include <uuid_v4.h>

using namespace vax;
using namespace vax::engine;
using namespace vax::vk;

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
        localTransform.updateRotationWithQuaternion(glm::quat(w, x, y, z));
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
PrefabLoader::loadPrefab(const PrefabDescriptor& descriptor) {
    switch (descriptor.getModelExtension()) {
    case PrefabDescriptor::ModelExtension::URDF:
        return _loadURDFPrefab(descriptor);
    case PrefabDescriptor::ModelExtension::GLB:
        return _loadGLBPrefab(descriptor);
    default:
        _logger.error("Unsupported model extension: ", descriptor.path);
        return std::nullopt;
    }
    return std::nullopt;
}

std::optional<std::pair<Prefab, std::vector<DrawableModel>>>
PrefabLoader::_loadURDFPrefab(const PrefabDescriptor& descriptor) {
    auto path = descriptor.path;
    auto model = urdf::parseURDFFile(path);
    if (!model) {
        _logger.error("Failed to load URDF model: " + path);
        return std::nullopt;
    }
    auto mainPath = descriptor.getMainPath();
    std::vector<DrawableModel> models;
    Prefab prefab;
    prefab.id = descriptor.id;
    _processURDFLink(_modelLoader, _resourceManager.get(), prefab, models, -1, mainPath, model->getRoot());
    return std::optional<std::pair<Prefab, std::vector<DrawableModel>>>(
        std::in_place, std::make_pair(std::move(prefab), std::move(models))
    );
}

std::optional<std::pair<Prefab, std::vector<DrawableModel>>>
PrefabLoader::_loadGLBPrefab(const PrefabDescriptor& descriptor) {
    auto model = _modelLoader.get().loadModel(descriptor.path);
    if (!model.has_value()) {
        _logger.error("Failed to load GLB model: " + descriptor.path);
        return std::nullopt;
    }
    std::vector<DrawableModel> models;
    models.push_back(std::move(*model));
    Prefab prefab{
        .id = descriptor.id,
        .nodes = {{.name = descriptor.id, .model = 0}},
    };
    return std::optional<std::pair<Prefab, std::vector<DrawableModel>>>(
        std::in_place, std::make_pair(std::move(prefab), std::move(models))
    );
}
