#pragma once

#include "colorPalette.h"
#include "jsonGlm.h"
#include "jsonOptional.h"
#include "transform.h"
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace vax::math {
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Transform, position, rotation, scale)
} // namespace vax::math

namespace vax::engine {
struct PrefabDescriptor {
    enum class PrefabType { ASSET, PRESET };

    enum class AssetExtension { GLB, URDF, UNKNOWN };

    enum class PrimitiveType { CUBE, PLANE, SPHERE, ICOSPHERE };

    struct CubePrimitiveDescriptor {
        float width = 1.0f;
        float height = 1.0f;
        float depth = 1.0f;
    };

    struct PlanePrimitiveDescriptor {
        float width = 1.0f;
        float height = 1.0f;
    };

    struct SpherePrimitiveDescriptor {
        float radius = 1.0f;
        uint32_t segments = 32;
        uint32_t rings = 16;
    };

    struct IcospherePrimitiveDescriptor {
        float radius = 1.0f;
        uint32_t subdivisions = 2;
    };

    using ShapeDescriptor = std::variant<
        CubePrimitiveDescriptor,
        PlanePrimitiveDescriptor,
        SpherePrimitiveDescriptor,
        IcospherePrimitiveDescriptor>;

    struct PrimitiveDescriptor {
        PrimitiveType primitiveType;
        std::optional<CubePrimitiveDescriptor> cubeDescriptor;
        std::optional<PlanePrimitiveDescriptor> planeDescriptor;
        std::optional<SpherePrimitiveDescriptor> sphereDescriptor;
        std::optional<IcospherePrimitiveDescriptor> icosphereDescriptor;
        Color color = ColorPalette::White;

        ShapeDescriptor getShapeDescriptor() const {
            switch (primitiveType) {
            case PrimitiveType::CUBE:
                return cubeDescriptor.value_or(CubePrimitiveDescriptor{});
            case PrimitiveType::PLANE:
                return planeDescriptor.value_or(PlanePrimitiveDescriptor{});
            case PrimitiveType::SPHERE:
                return sphereDescriptor.value_or(SpherePrimitiveDescriptor{});
            case PrimitiveType::ICOSPHERE:
                return icosphereDescriptor.value_or(IcospherePrimitiveDescriptor{});
            }
            return CubePrimitiveDescriptor{};
        }
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
    {PrefabDescriptor::PrimitiveType::SPHERE, "sphere"},
    {PrefabDescriptor::PrimitiveType::ICOSPHERE, "icosphere"},
    }
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(PrefabDescriptor::CubePrimitiveDescriptor, width, height, depth)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(PrefabDescriptor::PlanePrimitiveDescriptor, width, height)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(PrefabDescriptor::SpherePrimitiveDescriptor, radius, segments, rings)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(PrefabDescriptor::IcospherePrimitiveDescriptor, radius, subdivisions)

inline void to_json(nlohmann::json& json, const PrefabDescriptor::PrimitiveDescriptor& descriptor) {
    json = {
        {"primitiveType", descriptor.primitiveType},
        {"cubeDescriptor", descriptor.cubeDescriptor},
        {"planeDescriptor", descriptor.planeDescriptor},
        {"sphereDescriptor", descriptor.sphereDescriptor},
        {"icosphereDescriptor", descriptor.icosphereDescriptor},
        {"color", descriptor.color},
    };
}

inline void from_json(const nlohmann::json& json, PrefabDescriptor::PrimitiveDescriptor& descriptor) {
    PrefabDescriptor::PrimitiveDescriptor defaults;
    descriptor.cubeDescriptor =
        json.value("cubeDescriptor", std::optional<PrefabDescriptor::CubePrimitiveDescriptor>{});
    descriptor.planeDescriptor =
        json.value("planeDescriptor", std::optional<PrefabDescriptor::PlanePrimitiveDescriptor>{});
    descriptor.sphereDescriptor =
        json.value("sphereDescriptor", std::optional<PrefabDescriptor::SpherePrimitiveDescriptor>{});
    descriptor.icosphereDescriptor =
        json.value("icosphereDescriptor", std::optional<PrefabDescriptor::IcospherePrimitiveDescriptor>{});
    descriptor.color = json.value("color", defaults.color);

    if (json.contains("primitiveType")) {
        json.at("primitiveType").get_to(descriptor.primitiveType);
        return;
    }
    int filledCount = descriptor.cubeDescriptor.has_value() + descriptor.planeDescriptor.has_value() +
                      descriptor.sphereDescriptor.has_value() + descriptor.icosphereDescriptor.has_value();
    if (filledCount != 1) {
        throw nlohmann::json::other_error::create(
            501, "primitiveType is required unless exactly one shape descriptor is filled", &json
        );
    }
    if (descriptor.cubeDescriptor) {
        descriptor.primitiveType = PrefabDescriptor::PrimitiveType::CUBE;
    } else if (descriptor.planeDescriptor) {
        descriptor.primitiveType = PrefabDescriptor::PrimitiveType::PLANE;
    } else if (descriptor.sphereDescriptor) {
        descriptor.primitiveType = PrefabDescriptor::PrimitiveType::SPHERE;
    } else {
        descriptor.primitiveType = PrefabDescriptor::PrimitiveType::ICOSPHERE;
    }
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