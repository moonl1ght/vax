#pragma once

#include "colorPalette.h"
#include "jsonGlm.h"
#include "jsonOptional.h"
#include "materialConfig.h"
#include "physicsObjectConfig.h"
#include "transform.h"
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace vax::scene {
struct PrefabConfig final {
    enum class PrefabType { ASSET, PRESET };

    enum class AssetExtension { GLB, URDF, UNKNOWN };

    enum class PrimitiveType { CUBE, PLANE, SPHERE, ICOSPHERE };

    struct CubePrimitiveDescriptor final {
        float width = 1.0f;
        float height = 1.0f;
        float depth = 1.0f;
    };

    struct PlanePrimitiveDescriptor final {
        float width = 1.0f;
        float height = 1.0f;
    };

    struct SpherePrimitiveDescriptor final {
        float radius = 1.0f;
        uint32_t segments = 32;
        uint32_t rings = 16;
    };

    struct IcospherePrimitiveDescriptor final {
        float radius = 1.0f;
        uint32_t subdivisions = 2;
    };

    using ShapeDescriptor = std::variant<
        CubePrimitiveDescriptor,
        PlanePrimitiveDescriptor,
        SpherePrimitiveDescriptor,
        IcospherePrimitiveDescriptor>;

    struct PrimitiveDescriptor final {
        PrimitiveType primitiveType;
        std::optional<CubePrimitiveDescriptor> cubeDescriptor;
        std::optional<PlanePrimitiveDescriptor> planeDescriptor;
        std::optional<SpherePrimitiveDescriptor> sphereDescriptor;
        std::optional<IcospherePrimitiveDescriptor> icosphereDescriptor;
        gfx::Color color = gfx::ColorPalette::White;
        std::optional<MaterialConfig> materialConfig;

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

    struct AssetDescriptor final {
        std::string path;

        AssetExtension getAssetExtension() const;

        const std::string_view getMainPath() const;
    };

    struct InstanceInfo final {
        bool isSelected = false;
        vax::math::Transform transform;
        gfx::Color selectionColor = gfx::ColorPalette::Clear;
    };

    PrefabType prefabType;
    std::string id;
    std::vector<InstanceInfo> instanceInfos;
    std::optional<AssetDescriptor> assetDescriptor;
    std::optional<PrimitiveDescriptor> primitiveDescriptor;
    std::optional<PhysicsObjectConfig> physicsDescriptor;

    vax::math::Transform getTransformForInstance(uint32_t instanceIndex) const {
        if (instanceInfos.empty()) {
            return vax::math::Transform();
        }
        return instanceInfos[instanceIndex].transform;
    }
};

NLOHMANN_JSON_SERIALIZE_ENUM(
    PrefabConfig::PrefabType,
    {
    {PrefabConfig::PrefabType::ASSET, "asset"},
    {PrefabConfig::PrefabType::PRESET, "preset"},
    }
)

NLOHMANN_JSON_SERIALIZE_ENUM(
    PrefabConfig::PrimitiveType,
    {
    {PrefabConfig::PrimitiveType::CUBE, "cube"},
    {PrefabConfig::PrimitiveType::PLANE, "plane"},
    {PrefabConfig::PrimitiveType::SPHERE, "sphere"},
    {PrefabConfig::PrimitiveType::ICOSPHERE, "icosphere"},
    }
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(PrefabConfig::CubePrimitiveDescriptor, width, height, depth)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(PrefabConfig::PlanePrimitiveDescriptor, width, height)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(PrefabConfig::SpherePrimitiveDescriptor, radius, segments, rings)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(PrefabConfig::IcospherePrimitiveDescriptor, radius, subdivisions)

inline void to_json(nlohmann::json& json, const PrefabConfig::PrimitiveDescriptor& descriptor) {
    json = {
        {"primitiveType", descriptor.primitiveType},
        {"cubeDescriptor", descriptor.cubeDescriptor},
        {"planeDescriptor", descriptor.planeDescriptor},
        {"sphereDescriptor", descriptor.sphereDescriptor},
        {"icosphereDescriptor", descriptor.icosphereDescriptor},
        {"color", descriptor.color},
        {"materialConfig", descriptor.materialConfig},
    };
}

inline void from_json(const nlohmann::json& json, PrefabConfig::PrimitiveDescriptor& descriptor) {
    PrefabConfig::PrimitiveDescriptor defaults;
    descriptor.cubeDescriptor = json.value("cubeDescriptor", std::optional<PrefabConfig::CubePrimitiveDescriptor>{});
    descriptor.planeDescriptor = json.value("planeDescriptor", std::optional<PrefabConfig::PlanePrimitiveDescriptor>{});
    descriptor.sphereDescriptor =
        json.value("sphereDescriptor", std::optional<PrefabConfig::SpherePrimitiveDescriptor>{});
    descriptor.icosphereDescriptor =
        json.value("icosphereDescriptor", std::optional<PrefabConfig::IcospherePrimitiveDescriptor>{});
    descriptor.color = json.value("color", defaults.color);
    descriptor.materialConfig = json.value("materialConfig", std::optional<MaterialConfig>{});

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
        descriptor.primitiveType = PrefabConfig::PrimitiveType::CUBE;
    } else if (descriptor.planeDescriptor) {
        descriptor.primitiveType = PrefabConfig::PrimitiveType::PLANE;
    } else if (descriptor.sphereDescriptor) {
        descriptor.primitiveType = PrefabConfig::PrimitiveType::SPHERE;
    } else {
        descriptor.primitiveType = PrefabConfig::PrimitiveType::ICOSPHERE;
    }
}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PrefabConfig::AssetDescriptor, path)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(PrefabConfig::InstanceInfo, isSelected, transform, selectionColor)

inline void to_json(nlohmann::json& json, const PrefabConfig& config) {
    json = {
        {"prefabType", config.prefabType},
        {"id", config.id},
        {"instanceInfos", config.instanceInfos},
        {"assetDescriptor", config.assetDescriptor},
        {"primitiveDescriptor", config.primitiveDescriptor},
        {"physicsDescriptor", config.physicsDescriptor},
    };
}

inline void from_json(const nlohmann::json& json, PrefabConfig& config) {
    json.at("prefabType").get_to(config.prefabType);
    json.at("id").get_to(config.id);
    config.instanceInfos = json.value("instanceInfos", std::vector<PrefabConfig::InstanceInfo>{});
    if (config.instanceInfos.empty()) {
        config.instanceInfos.push_back(PrefabConfig::InstanceInfo{});
    }
    config.assetDescriptor = json.value("assetDescriptor", std::optional<PrefabConfig::AssetDescriptor>{});
    config.primitiveDescriptor = json.value("primitiveDescriptor", std::optional<PrefabConfig::PrimitiveDescriptor>{});
    config.physicsDescriptor = json.value("physicsDescriptor", std::optional<PhysicsObjectConfig>{});
}
} // namespace vax::scene