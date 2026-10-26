#pragma once

#include "colorPalette.h"
#include "jsonGlm.h"
#include "jsonOptional.h"
#include "physicsComponents.h"
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

    struct PhysicsDescriptor {
        enum class ColliderType { SPHERE, PLANE, NONE };

        struct SphereColliderDescriptor {
            float radius = 1.0f;

            ecs::SphereColliderComponent getSphereColliderComponent() const {
                return {
                    .radius = radius,
                };
            }
        };

        struct PlaneColliderDescriptor {
            float normalX = 0.0f;
            float normalY = 0.0f;
            float normalZ = 0.0f;
            float distance = 0.0f;

            ecs::PlaneColliderComponent getPlaneColliderComponent() const {
                return {
                    .normal = glm::vec3(normalX, normalY, normalZ),
                    .distance = distance,
                };
            }
        };

        ColliderType colliderType;
        float invMass = 0.0f;
        float restitution = 0.0f;
        float friction = 0.0f;
        bool isStatic = false;

        ecs::RigidBodyComponent getRigidBodyComponent() const {
            return {
                .invMass = invMass,
                .restitution = restitution,
                .friction = friction,
            };
        }

        std::optional<SphereColliderDescriptor> sphereColliderDescriptor;
        std::optional<PlaneColliderDescriptor> planeColliderDescriptor;
    };

    PrefabType prefabType;
    std::string id;
    std::vector<InstanceInfo> instanceInfos;
    std::optional<AssetDescriptor> assetDescriptor;
    std::optional<PrimitiveDescriptor> primitiveDescriptor;
    std::optional<PhysicsDescriptor> physicsDescriptor;

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

NLOHMANN_JSON_SERIALIZE_ENUM(
    PrefabDescriptor::PhysicsDescriptor::ColliderType,
    {
    {PrefabDescriptor::PhysicsDescriptor::ColliderType::SPHERE, "sphere"},
    {PrefabDescriptor::PhysicsDescriptor::ColliderType::PLANE, "plane"},
    {PrefabDescriptor::PhysicsDescriptor::ColliderType::NONE, "none"},
    }
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(PrefabDescriptor::PhysicsDescriptor::SphereColliderDescriptor, radius)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
    PrefabDescriptor::PhysicsDescriptor::PlaneColliderDescriptor, normalX, normalY, normalZ, distance
)

inline void to_json(nlohmann::json& json, const PrefabDescriptor::PhysicsDescriptor& descriptor) {
    json = {
        {"colliderType", descriptor.colliderType},
        {"invMass", descriptor.invMass},
        {"restitution", descriptor.restitution},
        {"friction", descriptor.friction},
        {"isStatic", descriptor.isStatic},
        {"sphereColliderDescriptor", descriptor.sphereColliderDescriptor},
        {"planeColliderDescriptor", descriptor.planeColliderDescriptor},
    };
}

inline void from_json(const nlohmann::json& json, PrefabDescriptor::PhysicsDescriptor& descriptor) {
    PrefabDescriptor::PhysicsDescriptor defaults;
    descriptor.invMass = json.value("invMass", defaults.invMass);
    descriptor.restitution = json.value("restitution", defaults.restitution);
    descriptor.friction = json.value("friction", defaults.friction);
    descriptor.isStatic = json.value("isStatic", defaults.isStatic);
    descriptor.sphereColliderDescriptor = json.value(
        "sphereColliderDescriptor", std::optional<PrefabDescriptor::PhysicsDescriptor::SphereColliderDescriptor>{}
    );
    descriptor.planeColliderDescriptor = json.value(
        "planeColliderDescriptor", std::optional<PrefabDescriptor::PhysicsDescriptor::PlaneColliderDescriptor>{}
    );

    if (json.contains("colliderType")) {
        json.at("colliderType").get_to(descriptor.colliderType);
        return;
    }
    int filledCount = descriptor.sphereColliderDescriptor.has_value() + descriptor.planeColliderDescriptor.has_value();
    if (filledCount != 1) {
        descriptor.colliderType = PrefabDescriptor::PhysicsDescriptor::ColliderType::NONE;
        return;
    }
    if (descriptor.sphereColliderDescriptor) {
        descriptor.colliderType = PrefabDescriptor::PhysicsDescriptor::ColliderType::SPHERE;
    } else if (descriptor.planeColliderDescriptor) {
        descriptor.colliderType = PrefabDescriptor::PhysicsDescriptor::ColliderType::PLANE;
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
        {"physicsDescriptor", descriptor.physicsDescriptor},
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
    descriptor.physicsDescriptor =
        json.value("physicsDescriptor", std::optional<PrefabDescriptor::PhysicsDescriptor>{});
}
} // namespace vax::engine