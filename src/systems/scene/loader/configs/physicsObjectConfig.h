#pragma once

#include "jsonOptional.h"
#include "physicsComponents.h"
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <optional>

namespace vax::scene {
struct PhysicsObjectConfig final {
    enum class ColliderType { SPHERE, PLANE, NONE };

    struct SphereColliderDescriptor final {
        float radius = 1.0f;

        ecs::SphereColliderComponent getSphereColliderComponent() const {
            return {
                .radius = radius,
            };
        }
    };

    struct PlaneColliderDescriptor final {
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
    float staticFriction = 0.0f;
    float dynamicFriction = 0.3f;
    bool isStatic = false;

    ecs::RigidBodyComponent getRigidBodyComponent() const {
        return {
            .invMass = invMass,
            .restitution = restitution,
            .staticFriction = staticFriction,
            .dynamicFriction = dynamicFriction,
        };
    }

    ecs::StaticRigidBodyComponent getStaticRigidBodyComponent() const {
        return {
            .restitution = restitution,
            .staticFriction = staticFriction,
            .dynamicFriction = dynamicFriction,
        };
    }

    std::optional<SphereColliderDescriptor> sphereColliderDescriptor;
    std::optional<PlaneColliderDescriptor> planeColliderDescriptor;
};

NLOHMANN_JSON_SERIALIZE_ENUM(
    PhysicsObjectConfig::ColliderType,
    {
    {PhysicsObjectConfig::ColliderType::SPHERE, "sphere"},
    {PhysicsObjectConfig::ColliderType::PLANE, "plane"},
    {PhysicsObjectConfig::ColliderType::NONE, "none"},
    }
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(PhysicsObjectConfig::SphereColliderDescriptor, radius)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
    PhysicsObjectConfig::PlaneColliderDescriptor, normalX, normalY, normalZ, distance
)

inline void to_json(nlohmann::json& json, const PhysicsObjectConfig& descriptor) {
    json = {
        {"colliderType", descriptor.colliderType},
        {"invMass", descriptor.invMass},
        {"restitution", descriptor.restitution},
        {"staticFriction", descriptor.staticFriction},
        {"dynamicFriction", descriptor.dynamicFriction},
        {"isStatic", descriptor.isStatic},
        {"sphereColliderDescriptor", descriptor.sphereColliderDescriptor},
        {"planeColliderDescriptor", descriptor.planeColliderDescriptor},
    };
}

inline void from_json(const nlohmann::json& json, PhysicsObjectConfig& descriptor) {
    PhysicsObjectConfig defaults;
    descriptor.invMass = json.value("invMass", defaults.invMass);
    descriptor.restitution = json.value("restitution", defaults.restitution);
    descriptor.staticFriction = json.value("staticFriction", defaults.staticFriction);
    descriptor.dynamicFriction = json.value("dynamicFriction", defaults.dynamicFriction);
    descriptor.isStatic = json.value("isStatic", defaults.isStatic);
    descriptor.sphereColliderDescriptor =
        json.value("sphereColliderDescriptor", std::optional<PhysicsObjectConfig::SphereColliderDescriptor>{});
    descriptor.planeColliderDescriptor =
        json.value("planeColliderDescriptor", std::optional<PhysicsObjectConfig::PlaneColliderDescriptor>{});

    if (json.contains("colliderType")) {
        json.at("colliderType").get_to(descriptor.colliderType);
        return;
    }
    int filledCount = descriptor.sphereColliderDescriptor.has_value() + descriptor.planeColliderDescriptor.has_value();
    if (filledCount != 1) {
        descriptor.colliderType = PhysicsObjectConfig::ColliderType::NONE;
        return;
    }
    if (descriptor.sphereColliderDescriptor) {
        descriptor.colliderType = PhysicsObjectConfig::ColliderType::SPHERE;
    } else if (descriptor.planeColliderDescriptor) {
        descriptor.colliderType = PhysicsObjectConfig::ColliderType::PLANE;
    }
}
} // namespace vax::scene
