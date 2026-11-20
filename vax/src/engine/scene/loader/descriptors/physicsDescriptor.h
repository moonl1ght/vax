#pragma once

#include "jsonOptional.h"
#include "physicsComponents.h"
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <optional>

namespace vax::engine {
struct PhysicsDescriptor final {
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
    PhysicsDescriptor::ColliderType,
    {
    {PhysicsDescriptor::ColliderType::SPHERE, "sphere"},
    {PhysicsDescriptor::ColliderType::PLANE, "plane"},
    {PhysicsDescriptor::ColliderType::NONE, "none"},
    }
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(PhysicsDescriptor::SphereColliderDescriptor, radius)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
    PhysicsDescriptor::PlaneColliderDescriptor, normalX, normalY, normalZ, distance
)

inline void to_json(nlohmann::json& json, const PhysicsDescriptor& descriptor) {
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

inline void from_json(const nlohmann::json& json, PhysicsDescriptor& descriptor) {
    PhysicsDescriptor defaults;
    descriptor.invMass = json.value("invMass", defaults.invMass);
    descriptor.restitution = json.value("restitution", defaults.restitution);
    descriptor.staticFriction = json.value("staticFriction", defaults.staticFriction);
    descriptor.dynamicFriction = json.value("dynamicFriction", defaults.dynamicFriction);
    descriptor.isStatic = json.value("isStatic", defaults.isStatic);
    descriptor.sphereColliderDescriptor =
        json.value("sphereColliderDescriptor", std::optional<PhysicsDescriptor::SphereColliderDescriptor>{});
    descriptor.planeColliderDescriptor =
        json.value("planeColliderDescriptor", std::optional<PhysicsDescriptor::PlaneColliderDescriptor>{});

    if (json.contains("colliderType")) {
        json.at("colliderType").get_to(descriptor.colliderType);
        return;
    }
    int filledCount = descriptor.sphereColliderDescriptor.has_value() + descriptor.planeColliderDescriptor.has_value();
    if (filledCount != 1) {
        descriptor.colliderType = PhysicsDescriptor::ColliderType::NONE;
        return;
    }
    if (descriptor.sphereColliderDescriptor) {
        descriptor.colliderType = PhysicsDescriptor::ColliderType::SPHERE;
    } else if (descriptor.planeColliderDescriptor) {
        descriptor.colliderType = PhysicsDescriptor::ColliderType::PLANE;
    }
}
} // namespace vax::engine
