#include "modelLoader.h"
#include "assetsLibrary.h"
#include "shaderSharedUtils.h"
#include <assimp/GltfMaterial.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <string>
#include <urdf_parser/urdf_parser.h>
#include <uuid_v4.h>

using namespace vax::engine;
using namespace vax;
using namespace vax::vk;

constexpr glm::mat4 toGlm(const aiMatrix4x4& m) {
    return {
        {m.a1, m.b1, m.c1, m.d1},
        {m.a2, m.b2, m.c2, m.d2},
        {m.a3, m.b3, m.c3, m.d3},
        {m.a4, m.b4, m.c4, m.d4},
    };
}

uint32_t loadTexture(aiString& textureName, vax::vk::TextureLoader& textureLoader, const aiScene* scene) {
    if (textureName.length > 0) {
        const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(textureName.C_Str());
        if (embeddedTexture) {
            void* textData = embeddedTexture->pcData;
            auto width = embeddedTexture->mWidth;
            auto height = embeddedTexture->mHeight;
            auto size = height == 0 ? width : width * height;
            auto data = std::span<unsigned char>(reinterpret_cast<unsigned char*>(textData), size);
            std::string name =
                std::string(scene->mRootNode->mName.C_Str()) + "_baseColorTexture_" + std::string(textureName.C_Str());
            auto texture = textureLoader.loadTexture(name, data);
            if (texture.has_value()) {
                return texture->first.id();
            }
            return NO_TEXTURE_FLAG;
        } else {
            auto texture = textureLoader.loadTexture(textureName.C_Str());
            if (texture.has_value()) {
                return texture->first.id();
            }
            return NO_TEXTURE_FLAG;
        }
    }
    return NO_TEXTURE_FLAG;
}

PBRMaterial processMaterial(
    aiMaterial* mat, vax::vk::TextureLoader& textureLoader, const aiScene* scene, vax::vk::SamplerId samplerId
) {
    PBRMaterial material{
        .baseColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
        .metallicFactor = 1.0f,
        .roughnessFactor = 1.0f,
        .normalScale = 1.0f,
        .occlusionStrength = 1.0f,
        .emissiveFactorAlphaCutoff = glm::vec4(0.0f, 0.0f, 0.0f, 0.5f),
        .baseColorTextureIndex = NO_TEXTURE_FLAG,
        .baseColorTextureSamplerIndex = NO_SAMPLER_INDEX,
        .baseColorTextureUVIndex = 0,
        .normalMapTextureIndex = NO_TEXTURE_FLAG,
        .normalMapTextureSamplerIndex = NO_SAMPLER_INDEX,
        .normalMapTextureUVIndex = 0,
        .metallicRoughnessTextureIndex = NO_TEXTURE_FLAG,
        .metallicRoughnessTextureSamplerIndex = NO_SAMPLER_INDEX,
        .metallicRoughnessTextureUVIndex = 0,
        .aoTextureIndex = NO_TEXTURE_FLAG,
        .aoTextureSamplerIndex = NO_SAMPLER_INDEX,
        .aoTextureUVIndex = 0,
        .emissiveTextureIndex = NO_TEXTURE_FLAG,
        .emissiveTextureSamplerIndex = NO_SAMPLER_INDEX,
        .emissiveTextureUVIndex = 0,
        .alphaMode = 0
    };
    aiColor4D color;
    ai_real factor;

    if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_BASE_COLOR, &color)) {
        material.baseColor = glm::vec4(color.r, color.g, color.b, color.a);
    }
    if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_EMISSIVE, &color)) {
        material.emissiveFactorAlphaCutoff = glm::vec4(color.r, color.g, color.b, 0.5f);
    }
    ai_real alphaCutoff;
    if (AI_SUCCESS == aiGetMaterialFloat(mat, AI_MATKEY_GLTF_ALPHACUTOFF, &alphaCutoff)) {
        material.emissiveFactorAlphaCutoff.w = alphaCutoff;
    }
    if (AI_SUCCESS == aiGetMaterialFloat(mat, AI_MATKEY_METALLIC_FACTOR, &factor)) {
        material.metallicFactor = factor;
    }
    if (AI_SUCCESS == aiGetMaterialFloat(mat, AI_MATKEY_ROUGHNESS_FACTOR, &factor)) {
        material.roughnessFactor = factor;
    }
    if (AI_SUCCESS == aiGetMaterialFloat(mat, AI_MATKEY_GLTF_TEXTURE_SCALE(aiTextureType_NORMALS, 0), &factor)) {
        material.normalScale = factor;
    }
    if (AI_SUCCESS ==
        aiGetMaterialFloat(mat, AI_MATKEY_GLTF_TEXTURE_SCALE(aiTextureType_AMBIENT_OCCLUSION, 0), &factor)) {
        material.occlusionStrength = factor;
    }

    aiString baseColorTextureName;
    aiString normalMapTextureName;
    aiString metallicRoughnessTextureName;
    aiString aoTextureName;
    aiString emissiveTextureName;

    mat->GetTexture(aiTextureType_BASE_COLOR, 0, &baseColorTextureName, nullptr, &material.baseColorTextureUVIndex);
    mat->GetTexture(aiTextureType_NORMALS, 0, &normalMapTextureName, nullptr, &material.normalMapTextureUVIndex);
    mat->GetTexture(
        aiTextureType_DIFFUSE_ROUGHNESS,
        0,
        &metallicRoughnessTextureName,
        nullptr,
        &material.metallicRoughnessTextureUVIndex
    );
    mat->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &aoTextureName, nullptr, &material.aoTextureUVIndex);
    mat->GetTexture(aiTextureType_EMISSIVE, 0, &emissiveTextureName, nullptr, &material.emissiveTextureUVIndex);

    material.baseColorTextureIndex = loadTexture(baseColorTextureName, textureLoader, scene);
    material.baseColorTextureSamplerIndex = samplerId;
    material.normalMapTextureIndex = loadTexture(normalMapTextureName, textureLoader, scene);
    material.normalMapTextureSamplerIndex = samplerId;
    material.metallicRoughnessTextureIndex = loadTexture(metallicRoughnessTextureName, textureLoader, scene);
    material.metallicRoughnessTextureSamplerIndex = samplerId;
    material.aoTextureIndex = loadTexture(aoTextureName, textureLoader, scene);
    material.aoTextureSamplerIndex = samplerId;
    material.emissiveTextureIndex = loadTexture(emissiveTextureName, textureLoader, scene);
    material.emissiveTextureSamplerIndex = samplerId;

    return material;
}

void processNode(
    const aiScene* scene,
    const aiNode* node,
    const glm::mat4& parentTransform,
    std::vector<Vertex>& vertices,
    std::vector<uint32_t>& indices,
    std::vector<Submesh>& submeshes,
    uint32_t& vertexOffset,
    uint32_t& indexOffset,
    uint32_t depth,
    std::vector<MaterialId>& materialIds,
    bool& hasTangents,
    const vax::Logger& logger
) {
    glm::mat4 transform = parentTransform * toGlm(node->mTransformation);
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));

    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

        Submesh submesh{
            .indexCount = mesh->mNumFaces * 3,
            .firstIndex = indexOffset,
            .vertexOffset = vertexOffset,
            .materialIndex = materialIds[mesh->mMaterialIndex]
        };
        submeshes.push_back(submesh);

        for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
            Vertex vertex;
            glm::vec4 pos = glm::vec4(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z, 1.0f);
            pos = transform * pos;
            vertex.position = glm::vec3(pos);
            vertex.normal = normalMatrix * glm::vec3(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z);
            if (mesh->HasTangentsAndBitangents()) {
                aiVector3D n = mesh->mNormals[v];
                aiVector3D t = mesh->mTangents[v];
                aiVector3D b = mesh->mBitangents[v];

                float w = ((n ^ t) * b < 0.0f) ? -1.0f : 1.0f;
                glm::vec3 tangentWorld = normalMatrix * glm::vec3(t.x, t.y, t.z);
                vertex.tangent = glm::vec4(tangentWorld, w);
                hasTangents = true;
            }
            if (mesh->mTextureCoords[0]) {
                vertex.uv = {mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y};
            }
            if (mesh->mTextureCoords[1]) {
                vertex.uv2 = {mesh->mTextureCoords[1][v].x, mesh->mTextureCoords[1][v].y};
            }
            if (mesh->mColors[0]) {
                glm::vec4 color = glm::vec4(
                    mesh->mColors[0][v].r, mesh->mColors[0][v].g, mesh->mColors[0][v].b, mesh->mColors[0][v].a
                );
                vertex.packedColor = packRGBA(color);
            }
            vertices.push_back(vertex);
        }

        for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
            const aiFace& face = mesh->mFaces[f];
            for (unsigned int j = 0; j < face.mNumIndices; ++j) {
                indices.push_back(face.mIndices[j]);
            }
        }

        vertexOffset += mesh->mNumVertices;
        indexOffset += submesh.indexCount;
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        processNode(
            scene,
            node->mChildren[i],
            transform,
            vertices,
            indices,
            submeshes,
            vertexOffset,
            indexOffset,
            depth + 1,
            materialIds,
            hasTangents,
            logger
        );
    }
}

std::optional<DrawableModel> ModelLoader::loadModel(const std::string& path, uint32_t instancesCount) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        _logger.error("Failed to load model: " + std::string(importer.GetErrorString()));
        return std::nullopt;
    }
    size_t totalVertexCount = 0;
    size_t totalIndexCount = 0;

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        totalVertexCount += scene->mMeshes[i]->mNumVertices;
        totalIndexCount += scene->mMeshes[i]->mNumFaces * 3;
    }

    auto pbrSamplerId = static_cast<vax::vk::SamplerId>(vax::vk::GlobalSampler::PBRSampler);

    std::vector<Vertex> modelVertices;
    modelVertices.reserve(totalVertexCount);

    std::vector<uint32_t> modelIndices;
    modelIndices.reserve(totalIndexCount);

    std::vector<Submesh> submeshes;
    uint32_t currentVertexOffset = 0;
    uint32_t currentIndexOffset = 0;

    submeshes.reserve(scene->mNumMeshes);

    std::vector<PBRMaterial> materials;
    materials.reserve(scene->mNumMaterials);
    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        materials.push_back(processMaterial(scene->mMaterials[i], *_textureLoader, scene, pbrSamplerId));
    }

    auto materialIds = _resourceManager.get().materialManager().insertMaterials(materials);
    if (materialIds.size() != materials.size()) {
        _logger.error("Failed to load materials");
        return std::nullopt;
    }

    bool hasTangents = false;
    processNode(
        scene,
        scene->mRootNode,
        glm::mat4(1.0f),
        modelVertices,
        modelIndices,
        submeshes,
        currentVertexOffset,
        currentIndexOffset,
        0,
        materialIds,
        hasTangents,
        _logger
    );

    auto mesh = _resourceManager.get().meshManager().createEmptyMesh();
    if (!mesh)
        return std::nullopt;

    (*mesh).second->setName(path);
    (*mesh).second->setVertices(modelVertices);
    (*mesh).second->setIndices(modelIndices);
    (*mesh).second->lock();
    (*mesh).second->bindBuffers();
    if (!(*mesh).second->flushToGPU()) {
        _logger.error("Failed to flush mesh to GPU");
        return std::nullopt;
    }

    auto drawableModel = vax::engine::DrawableModel(_resourceManager.get().meshManager(), mesh->first);
    drawableModel._mesh = (*mesh).second;
    drawableModel._submeshes = submeshes;
    drawableModel._settings.hasTangents = hasTangents;
    return std::optional<DrawableModel>(std::in_place, std::move(drawableModel));
}

static glm::mat4 urdfPoseToMat4(const urdf::Pose& pose) {
    glm::vec3 pos(pose.position.x, pose.position.y, pose.position.z);
    glm::quat rot(
        static_cast<float>(pose.rotation.w),
        static_cast<float>(pose.rotation.x),
        static_cast<float>(pose.rotation.y),
        static_cast<float>(pose.rotation.z)
    );
    return glm::translate(glm::mat4(1.0f), pos) * glm::mat4_cast(rot);
}

template <typename ModelLoaderFunc>
DrawableNode processURDFLink(
    const std::string_view mainPath,
    vax::vk::ResourceManager& resourceManager,
    urdf::LinkConstSharedPtr link,
    ModelLoaderFunc loadModel,
    const glm::mat4& parentTransform = glm::mat4(1.0f)
) {
    auto transformHandle = vax::math::TransformHandle();
    if (link->parent_joint) {
        auto pose = link->parent_joint->parent_to_joint_origin_transform;
        transformHandle.setPosition(glm::vec3(pose.position.x, pose.position.y, pose.position.z));
        double quatX, quatY, quatZ, quatW;
        pose.rotation.getQuaternion(quatX, quatY, quatZ, quatW);
        glm::quat quat(quatW, quatX, quatY, quatZ);
        transformHandle.setRotation(glm::eulerAngles(quat));
    }

    auto transform = transformHandle.getModelMatrix();
    auto nodeTransform = parentTransform * transform;

    DrawableNode node(
        resourceManager.ssboManager(), link->name, transformHandle.getTransform(), {nodeTransform}, !link->parent_joint
    );

    for (const auto& visual : link->visual_array) {
        if (!visual || !visual->geometry)
            continue;

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
        if (visual->geometry->type == urdf::Geometry::MESH) {
            const auto* mesh = static_cast<const urdf::Mesh*>(visual->geometry.get());
            auto modelOpt = loadModel(std::string(mainPath) + "/" + mesh->filename);
            if (modelOpt.has_value()) {
                for (size_t i = 0; i < modelOpt->drawableModel->submeshCount(); ++i) {
                    modelOpt->drawableModel->submesh(i).materialIndex = materialId;
                }
                node.addDrawableModel(std::move(*modelOpt));
            }
        }
    }

    for (const auto& child : link->child_links) {
        auto childNode = processURDFLink(mainPath, resourceManager, child, loadModel, nodeTransform);
        node.insertChild(std::make_unique<DrawableNode>(std::move(childNode)));
    }

    return node;
}

std::optional<DrawableNode>
ModelLoader::_loadURDFSceneModel(AssetsLibrary& assetsLibrary, PrefabDescriptor descriptor) {
    auto path = descriptor.path;
    auto model = urdf::parseURDFFile(path);
    if (!model) {
        _logger.error("Failed to load URDF model: " + path);
        return std::nullopt;
    }
    auto mainPath = descriptor.getMainPath();
    auto rootNode = processURDFLink(
        mainPath,
        _resourceManager.get(),
        model->getRoot(),
        [&](std::string path) -> std::optional<DrawableModelHandle> {
            auto model = loadModel(path, 1);
            if (!model.has_value()) {
                _logger.error("Failed to load model: " + path);
                return std::nullopt;
            }
            static UUIDv4::UUIDGenerator<std::mt19937_64> uuidGen;
            auto drawableModelHandle =
                assetsLibrary._addDrawableModel(uuidGen.getUUID().str(), std::move(*model));
            return std::optional<DrawableModelHandle>(std::in_place, std::move(drawableModelHandle));
        }
    );
    rootNode.updateTransforms([&](uint32_t instanceIndex, math::TransformHandle& transformHandle) {
        transformHandle.updateTransform([&](math::Transform& transform) {
            transform = descriptor.transforms[instanceIndex];
        });
    });
    return std::optional<DrawableNode>(std::in_place, std::move(rootNode));
}

std::optional<DrawableNode>
ModelLoader::_loadGLBSceneModel(AssetsLibrary& assetsLibrary, PrefabDescriptor descriptor) {
    auto path = descriptor.path;
    auto model = loadModel(path, 1);
    if (!model.has_value()) {
        _logger.error("Failed to load GLB model: " + path);
        return std::nullopt;
    }
    auto transformHandle = vax::math::TransformHandle();
    auto node = DrawableNode(
        _resourceManager.get().ssboManager(),
        path,
        transformHandle.getTransform(),
        {transformHandle.getModelMatrix()},
        true
    );
    static UUIDv4::UUIDGenerator<std::mt19937_64> uuidGen;
    auto drawableModel = assetsLibrary._addDrawableModel(uuidGen.getUUID().str(), std::move(*model));
    node.addDrawableModel(drawableModel);
    return std::optional<DrawableNode>(std::in_place, std::move(node));
}

std::optional<DrawableNode>
ModelLoader::loadSceneModel(const vax::engine::PrefabDescriptor& descriptor) {
    if (descriptor.getModelExtension() == vax::engine::PrefabDescriptor::ModelExtension::URDF) {
        return _loadURDFSceneModel(_assetsLibrary.get(), descriptor);
    } else if (descriptor.getModelExtension() == vax::engine::PrefabDescriptor::ModelExtension::GLB) {
        return _loadGLBSceneModel(_assetsLibrary.get(), descriptor);
    }
    return std::nullopt;
}