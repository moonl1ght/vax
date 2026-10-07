#pragma once

#include "colorPalette.h"
#include "transform.h"

namespace vax::engine {
struct PrefabDescriptor {
    enum class PrefabType { ASSET, PRESET };

    enum class AssetExtension { GLB, URDF, UNKNOWN };

    enum class PrimitiveType { CUBE, PLANE };

    struct PrimitiveDescriptor {
        PrimitiveType primitiveType;
        float size = 1.0f;
        Color color = ColorPalette::White;
    };

    struct AssetDescriptor {
        std::string path;

        AssetExtension getAssetExtension() const;

        const std::string_view getMainPath() const;
    };

    struct SelectedInstanceInfo {
        uint32_t instanceIndex;
        Color color = ColorPalette::Clear;
    };

    std::string id;
    std::vector<vax::math::Transform> transforms;
    std::vector<SelectedInstanceInfo> selectedInstanceInfos;
    std::optional<AssetDescriptor> assetDescriptor;
    std::optional<PrimitiveDescriptor> primitiveDescriptor;
    uint32_t instancesCount;
    bool isIdentifiable;
    PrefabType prefabType;
};
} // namespace vax::engine