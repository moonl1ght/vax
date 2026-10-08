#pragma once

#include "colorPalette.h"
#include "jsonGlm.h"
#include "jsonOptional.h"
#include "transform.h"
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

namespace vax::math {
// rotation is in radians, same as in Transform
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Transform, position, rotation, scale)
} // namespace vax::math

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

    struct InstanceInfo {
        bool isSelected = false;
        vax::math::Transform transform;
        Color selectionColor = ColorPalette::Clear;
    };

    PrefabType prefabType;
    std::string id;
    std::vector<InstanceInfo> instanceInfos;
    std::optional<AssetDescriptor> assetDescriptor;
    std::optional<PrimitiveDescriptor> primitiveDescriptor;

    vax::math::Transform getTransformForInstance(uint32_t instanceIndex) const {
        if (instanceInfos.empty()) {
            return vax::math::Transform();
        }
        return instanceInfos[instanceIndex].transform;
    }
};

NLOHMANN_JSON_SERIALIZE_ENUM(
    PrefabDescriptor::PrefabType,
    {
    {PrefabDescriptor::PrefabType::ASSET, "asset"},
    {PrefabDescriptor::PrefabType::PRESET, "preset"},
    }
)

NLOHMANN_JSON_SERIALIZE_ENUM(
    PrefabDescriptor::PrimitiveType,
    {
    {PrefabDescriptor::PrimitiveType::CUBE, "cube"},
    {PrefabDescriptor::PrimitiveType::PLANE, "plane"},
    }
)

inline void to_json(nlohmann::json& json, const PrefabDescriptor::PrimitiveDescriptor& descriptor) {
    json = {{"primitiveType", descriptor.primitiveType}, {"size", descriptor.size}, {"color", descriptor.color}};
}

inline void from_json(const nlohmann::json& json, PrefabDescriptor::PrimitiveDescriptor& descriptor) {
    PrefabDescriptor::PrimitiveDescriptor defaults;
    json.at("primitiveType").get_to(descriptor.primitiveType);
    descriptor.size = json.value("size", defaults.size);
    descriptor.color = json.value("color", defaults.color);
}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PrefabDescriptor::AssetDescriptor, path)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(PrefabDescriptor::InstanceInfo, isSelected, transform, selectionColor)

inline void to_json(nlohmann::json& json, const PrefabDescriptor& descriptor) {
    json = {
        {"prefabType", descriptor.prefabType},
        {"id", descriptor.id},
        {"instanceInfos", descriptor.instanceInfos},
        {"assetDescriptor", descriptor.assetDescriptor},
        {"primitiveDescriptor", descriptor.primitiveDescriptor},
    };
}

inline void from_json(const nlohmann::json& json, PrefabDescriptor& descriptor) {
    json.at("prefabType").get_to(descriptor.prefabType);
    json.at("id").get_to(descriptor.id);
    descriptor.instanceInfos = json.value("instanceInfos", std::vector<PrefabDescriptor::InstanceInfo>{});
    if (descriptor.instanceInfos.empty()) {
        descriptor.instanceInfos.push_back(PrefabDescriptor::InstanceInfo{});
    }
    descriptor.assetDescriptor = json.value("assetDescriptor", std::optional<PrefabDescriptor::AssetDescriptor>{});
    descriptor.primitiveDescriptor =
        json.value("primitiveDescriptor", std::optional<PrefabDescriptor::PrimitiveDescriptor>{});
}
} // namespace vax::engine