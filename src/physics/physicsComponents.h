#pragma once

#include "entity.h"
#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <limits>
#include <optional>
#include <type_traits>
#include <variant>
#include <vector>

namespace vax::ecs {

struct RigidBodyComponent final {
    float invMass = 0.0f; // inverse mass, 1/mass
    float restitution = 0.0f;
    float staticFriction = 0.0f;
    float dynamicFriction = 0.3f;
    float linearDamping = 0.0f;
    float angularDamping = 0.0f;
    float gravityScale = 1.0f;

    // Better to set StaticRigidBodyComponent instead of invMass == 0.0f
    bool isStatic() const noexcept { return invMass == 0.0f; }
}; // 28 bytes

// if entity has RigidBodyComponent and StaticRigidBodyComponent, it treats as static
struct StaticRigidBodyComponent final {};

struct EmptyColliderComponent final {};

struct SphereColliderComponent final {
    float radius;
}; // 4 bytes

struct PlaneColliderComponent final {
    glm::vec3 normal;
    float distance;
}; // 16 bytes

struct CapsuleColliderComponent final {
    float radius;
    float height;
}; // 8 bytes

struct BoxColliderComponent final {
    glm::vec3 size;
}; // 12 bytes

struct CylinderColliderComponent final {
    float radius;
    float height;
}; // 8 bytes

} // namespace vax::ecs

namespace vax::physics {

using ShapeCollider = std::variant<
    ecs::EmptyColliderComponent,
    ecs::SphereColliderComponent,
    ecs::PlaneColliderComponent,
    ecs::CapsuleColliderComponent,
    ecs::BoxColliderComponent,
    ecs::CylinderColliderComponent>;

struct AABB final {
    glm::vec3 min;
    glm::vec3 max;

    AABB(const glm::vec3& min, const glm::vec3& max) noexcept
        : min(min)
        , max(max) {};

    static AABB sphere(const glm::vec3& center, float radius) noexcept {
        return AABB(center - glm::vec3(radius), center + glm::vec3(radius));
    }

    static AABB plane(const glm::vec3& normal, float distance) noexcept {
        return AABB(
            glm::vec3(-std::numeric_limits<float>::infinity()), glm::vec3(std::numeric_limits<float>::infinity())
        );
    }

    static AABB capsule(const glm::vec3& center, const glm::quat& orientation, float radius, float height) noexcept {
        const float halfSegment = glm::max(height * 0.5f - radius, 0.0f);
        const glm::vec3 axis = orientation * glm::vec3(0.0f, 1.0f, 0.0f);
        const glm::vec3 extent = glm::abs(axis) * halfSegment + glm::vec3(radius);
        return AABB(center - extent, center + extent);
    }

    static AABB box(const glm::vec3& center, const glm::quat& orientation, const glm::vec3& size) noexcept {
        const glm::vec3 halfExtents = size * 0.5f;
        const glm::mat3 rotation = glm::mat3_cast(orientation);
        const glm::vec3 extent = glm::abs(rotation[0]) * halfExtents.x + glm::abs(rotation[1]) * halfExtents.y +
                                 glm::abs(rotation[2]) * halfExtents.z;
        return AABB(center - extent, center + extent);
    }

    static AABB cylinder(const glm::vec3& center, const glm::quat& orientation, float radius, float height) noexcept {
        const glm::vec3 axis = orientation * glm::vec3(0.0f, 1.0f, 0.0f);
        const glm::vec3 discExtent = glm::sqrt(glm::max(glm::vec3(1.0f) - axis * axis, glm::vec3(0.0f))) * radius;
        const glm::vec3 extent = glm::abs(axis) * (height * 0.5f) + discExtent;
        return AABB(center - extent, center + extent);
    }

    static std::optional<AABB>
    getAABB(const ShapeCollider& shapeCollider, const glm::vec3& position, const glm::quat& orientation) noexcept {
        return std::visit(
            [&](const auto& shapeColliderComponent) -> std::optional<AABB> {
                using T = std::decay_t<decltype(shapeColliderComponent)>;
                if constexpr (std::is_same_v<T, ecs::SphereColliderComponent>) {
                    return AABB::sphere(position, shapeColliderComponent.radius);
                } else if constexpr (std::is_same_v<T, ecs::PlaneColliderComponent>) {
                    return AABB::plane(shapeColliderComponent.normal, shapeColliderComponent.distance);
                } else if constexpr (std::is_same_v<T, ecs::CapsuleColliderComponent>) {
                    return AABB::capsule(
                        position, orientation, shapeColliderComponent.radius, shapeColliderComponent.height
                    );
                } else if constexpr (std::is_same_v<T, ecs::BoxColliderComponent>) {
                    return AABB::box(position, orientation, shapeColliderComponent.size);
                } else if constexpr (std::is_same_v<T, ecs::CylinderColliderComponent>) {
                    return AABB::cylinder(
                        position, orientation, shapeColliderComponent.radius, shapeColliderComponent.height
                    );
                }
                return std::nullopt;
            },
            shapeCollider
        );
    }

    inline bool intersects(const AABB& other) const noexcept {
        return this->min.x <= other.max.x && other.min.x <= this->max.x && this->min.y <= other.max.y &&
               other.min.y <= this->max.y && this->min.z <= other.max.z && other.min.z <= this->max.z;
    }
};

struct BodyCollider final {
    constexpr static uint32_t NullBodyIndex = std::numeric_limits<uint32_t>::max();

    ShapeCollider shapeCollider;
    uint32_t bodyIndex = NullBodyIndex;
};

struct StaticBody final {
    glm::vec3 position;
    glm::quat orientation;

    float restitution = 0.0f;
    float staticFriction = 0.5f;
    float dynamicFriction = 0.3f;

    ecs::Entity entity;
};

struct DynamicBody final {
    glm::vec3 position;
    glm::vec3 previousPosition;
    glm::vec3 velocity;

    // glm::quat orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    // glm::quat previousOrientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    // glm::vec3 angularVelocity = glm::vec3(0.0f);

    // glm::vec3 invInertiaLocal = glm::vec3(0.0f);
    // glm::mat3 invInertiaWorld = glm::mat3(0.0f);

    ecs::Entity entity;

    ecs::RigidBodyComponent rigidBodyComponent;
};

struct BodyStorage final {
    std::vector<DynamicBody> dynamicBodies;
    std::vector<BodyCollider> dynamicBodyColliders;
    std::vector<StaticBody> staticBodies;
    std::vector<BodyCollider> staticBodyColliders;

    template <typename ShapeType> void addDynamicBody(const DynamicBody& dynamicBody, const ShapeType& shape) noexcept {
        dynamicBodyColliders.push_back(
            BodyCollider{.shapeCollider = shape, .bodyIndex = static_cast<uint32_t>(dynamicBodies.size())}
        );
        dynamicBodies.push_back(dynamicBody);
    }

    template <typename ShapeType> void addStaticBody(const StaticBody& staticBody, const ShapeType& shape) noexcept {
        staticBodyColliders.push_back(
            BodyCollider{.shapeCollider = shape, .bodyIndex = static_cast<uint32_t>(staticBodies.size())}
        );
        staticBodies.push_back(staticBody);
    }

    void clear() noexcept {
        dynamicBodies.clear();
        dynamicBodyColliders.clear();
        staticBodies.clear();
        staticBodyColliders.clear();
    }
};

struct ContactConstraint final {
    ecs::Entity entityA;
    ecs::Entity entityB;
    glm::vec3 point;
    glm::vec3 normal;
    float penetrationDepth;
    float compliance = 0.0f;
    float lagrangeMultiplier = 0.0f;
};

} // namespace vax::physics