#include "collisionDetector.h"
#include "physicsComponents.h"

using namespace vax::physics;

struct ColliderView final {
    uint32_t bodyIndex;
    const glm::vec3& position;
    const glm::quat& orientation;
    const ShapeCollider& shape;
    float restitution;
    float staticFriction;
    float dynamicFriction;
};

std::optional<ContactConstraint> sphereToSphere(
    uint32_t indexFirst,
    const glm::vec3& positionFirst,
    float radiusFirst,
    float restitutionFirst,
    float staticFrictionFirst,
    float dynamicFrictionFirst,
    uint32_t indexSecond,
    const glm::vec3& positionSecond,
    float radiusSecond,
    float restitutionSecond,
    float staticFrictionSecond,
    float dynamicFrictionSecond,
    bool isSecondStatic
) noexcept {
    const glm::vec3 delta = positionSecond - positionFirst;
    const float lengthSquared = glm::dot(delta, delta);
    const float radiusSum = radiusFirst + radiusSecond;

    if (lengthSquared >= radiusSum * radiusSum) [[unlikely]] {
        return std::nullopt;
    }

    const float length = std::sqrt(lengthSquared);

    const glm::vec3 normal = length > CollisionDetector::EPSILON ? delta / length : glm::vec3(0.0f, 1.0f, 0.0f);

    return ContactConstraint{
        .first = indexFirst,
        .second = indexSecond,
        .secondIsStatic = isSecondStatic,
        .normalFromFirstToSecond = normal,
        .localPointFirst = normal * radiusFirst,
        .localPointSecond = -normal * radiusSecond,
        .penetrationDepth = radiusSum - length,
        .staticFriction = 0.5f * (staticFrictionFirst + staticFrictionSecond),
        .dynamicFriction = 0.5f * (dynamicFrictionFirst + dynamicFrictionSecond),
        .restitution = std::max(restitutionFirst, restitutionSecond),
        .normalVelocityBefore = 0.0f,
    };
}

std::optional<ContactConstraint> sphereToPlane(
    uint32_t indexSphere,
    const glm::vec3& positionSphere,
    float radiusSphere,
    float restitutionSphere,
    float staticFrictionSphere,
    float dynamicFrictionSphere,
    uint32_t indexPlane,
    const glm::vec3& positionPlane,
    const glm::quat& orientationPlane,
    const glm::vec3& normalPlane,
    float distancePlane,
    float restitutionPlane,
    float staticFrictionPlane,
    float dynamicFrictionPlane,
    bool isPlaneStatic
) noexcept {
    const glm::vec3 planeNormal = glm::normalize(orientationPlane * normalPlane);
    const float planeOffset = distancePlane + glm::dot(planeNormal, positionPlane);
    const float signedDistance = glm::dot(planeNormal, positionSphere) - planeOffset;

    if (signedDistance >= radiusSphere) [[unlikely]] {
        return std::nullopt;
    }

    const glm::vec3 normal = -planeNormal;
    const glm::vec3 contactPointOnPlane = positionSphere - planeNormal * signedDistance;

    return ContactConstraint{
        .first = indexSphere,
        .second = indexPlane,
        .secondIsStatic = isPlaneStatic,
        .normalFromFirstToSecond = normal,
        .localPointFirst = normal * radiusSphere,
        .localPointSecond = contactPointOnPlane - positionPlane,
        .penetrationDepth = radiusSphere - signedDistance,
        .staticFriction = 0.5f * (staticFrictionSphere + staticFrictionPlane),
        .dynamicFriction = 0.5f * (dynamicFrictionSphere + dynamicFrictionPlane),
        .restitution = std::max(restitutionSphere, restitutionPlane),
        .normalVelocityBefore = 0.0f,
    };
}

void flipContact(ContactConstraint& contact, bool isNewSecondStatic) noexcept {
    std::swap(contact.first, contact.second);
    std::swap(contact.localPointFirst, contact.localPointSecond);
    contact.normalFromFirstToSecond = -contact.normalFromFirstToSecond;
    contact.secondIsStatic = isNewSecondStatic;
}

std::optional<ContactConstraint>
sphereToSphere(const ColliderView& a, const ColliderView& b, bool isSecondStatic) noexcept {
    return sphereToSphere(
        a.bodyIndex,
        a.position,
        std::get<vax::ecs::SphereColliderComponent>(a.shape).radius,
        a.restitution,
        a.staticFriction,
        a.dynamicFriction,
        b.bodyIndex,
        b.position,
        std::get<vax::ecs::SphereColliderComponent>(b.shape).radius,
        b.restitution,
        b.staticFriction,
        b.dynamicFriction,
        isSecondStatic
    );
}

std::optional<ContactConstraint>
sphereToPlane(const ColliderView& sphere, const ColliderView& plane, bool isPlaneStatic) noexcept {
    const auto& planeShape = std::get<vax::ecs::PlaneColliderComponent>(plane.shape);
    return sphereToPlane(
        sphere.bodyIndex,
        sphere.position,
        std::get<vax::ecs::SphereColliderComponent>(sphere.shape).radius,
        sphere.restitution,
        sphere.staticFriction,
        sphere.dynamicFriction,
        plane.bodyIndex,
        plane.position,
        plane.orientation,
        planeShape.normal,
        planeShape.distance,
        plane.restitution,
        plane.staticFriction,
        plane.dynamicFriction,
        isPlaneStatic
    );
}

void CollisionDetector::detectCollisions(const BodyStorage& bodyStorage) noexcept {
    auto dynamicView = [&](uint32_t aabbIndex) {
        const auto& collider = bodyStorage.dynamicBodyColliders[_dynamicAABBs[aabbIndex].colliderIndex];
        const auto& body = bodyStorage.dynamicBodies[collider.bodyIndex];
        return ColliderView{
            collider.bodyIndex,
            body.position,
            body.orientation,
            collider.shapeCollider,
            body.rigidBodyComponent.restitution,
            body.rigidBodyComponent.staticFriction,
            body.rigidBodyComponent.dynamicFriction,
        };
    };
    auto staticView = [&](uint32_t aabbIndex) {
        const auto& collider = bodyStorage.staticBodyColliders[_staticAABBs[aabbIndex].colliderIndex];
        const auto& body = bodyStorage.staticBodies[collider.bodyIndex];
        return ColliderView{
            collider.bodyIndex,
            body.position,
            body.orientation,
            collider.shapeCollider,
            body.restitution,
            body.staticFriction,
            body.dynamicFriction,
        };
    };
    auto addContact = [&](std::optional<ContactConstraint> contact) {
        if (!contact) {
            return;
        }
        const glm::vec3& firstVelocity = bodyStorage.dynamicBodies[contact->first].velocity;
        const glm::vec3& secondVelocity =
            contact->secondIsStatic ? glm::vec3(0.0f) : bodyStorage.dynamicBodies[contact->second].velocity;
        contact->normalVelocityBefore = glm::dot(firstVelocity - secondVelocity, contact->normalFromFirstToSecond);
        _contactConstraints.push_back(*contact);
    };
    auto dts = [&](CollisionType type) -> const CollisionBucket& {
        return _dynamicToStaticCollisions[static_cast<uint32_t>(type)];
    };
    auto dtd = [&](CollisionType type) -> const CollisionBucket& {
        return _dynamicToDynamicCollisions[static_cast<uint32_t>(type)];
    };

    _contactConstraints.clear();

    for (const auto& pair : dts(CollisionType::SphereToSphere)) {
        addContact(sphereToSphere(dynamicView(pair.first), staticView(pair.second), true));
    }
    for (const auto& pair : dtd(CollisionType::SphereToSphere)) {
        addContact(sphereToSphere(dynamicView(pair.first), dynamicView(pair.second), false));
    }
    for (const auto& pair : dts(CollisionType::SphereToPlane)) {
        addContact(sphereToPlane(dynamicView(pair.first), staticView(pair.second), true));
    }
    for (const auto& pair : dtd(CollisionType::SphereToPlane)) {
        addContact(sphereToPlane(dynamicView(pair.first), dynamicView(pair.second), false));
    }
    for (const auto& pair : dtd(CollisionType::PlaneToSphere)) {
        addContact(sphereToPlane(dynamicView(pair.second), dynamicView(pair.first), false));
    }
    for (const auto& pair : dts(CollisionType::PlaneToSphere)) {
        auto contact = sphereToPlane(staticView(pair.second), dynamicView(pair.first), false);
        if (contact) {
            flipContact(*contact, true);
        }
        addContact(contact);
    }
}

void CollisionDetector::buildDynamicAABB(const BodyStorage& bodyStorage, float dt) noexcept {
    _dynamicAABBs.clear();
    _dynamicAABBs.reserve(bodyStorage.dynamicBodyColliders.size());
    for (size_t i = 0; i < bodyStorage.dynamicBodyColliders.size(); ++i) {
        auto& collider = bodyStorage.dynamicBodyColliders[i];
        const auto& body = bodyStorage.dynamicBodies[collider.bodyIndex];
        auto aabb = AABB::getAABB(collider.shapeCollider, body.position, body.orientation);
        if (!aabb) {
            continue;
        }
        const glm::vec3 delta = body.velocity * dt;
        aabb->min += glm::min(delta, glm::vec3(0.0f));
        aabb->max += glm::max(delta, glm::vec3(0.0f));
        _dynamicAABBs.push_back({*aabb, static_cast<uint32_t>(i)});
    }
}

void CollisionDetector::buildStaticAABB(const BodyStorage& bodyStorage) noexcept {
    _staticAABBs.clear();
    _staticAABBs.reserve(bodyStorage.staticBodyColliders.size());
    for (size_t i = 0; i < bodyStorage.staticBodyColliders.size(); ++i) {
        auto& collider = bodyStorage.staticBodyColliders[i];
        const auto& body = bodyStorage.staticBodies[collider.bodyIndex];
        auto aabb = AABB::getAABB(collider.shapeCollider, body.position, body.orientation);
        if (!aabb) {
            continue;
        }
        _staticAABBs.push_back({*aabb, static_cast<uint32_t>(i)});
    }
}

void CollisionDetector::findCollisionPairs(const BodyStorage& bodyStorage) noexcept {
    auto processCollision = [&](std::array<CollisionBucket, NUM_COLLISION_TYPES>& collisionsStorage,
                                const auto& shapeA,
                                const auto& shapeB,
                                uint32_t indexA,
                                uint32_t indexB) {
        std::visit(
            [&](const auto& shapeA, const auto& shapeB) {
                using AType = std::decay_t<decltype(shapeA)>;
                using BType = std::decay_t<decltype(shapeB)>;
                if constexpr (
                    std::is_same_v<AType, ecs::SphereColliderComponent> &&
                    std::is_same_v<BType, ecs::SphereColliderComponent>
                ) {
                    collisionsStorage[static_cast<uint32_t>(CollisionType::SphereToSphere)].push_back({indexA, indexB});
                } else if constexpr (
                    std::is_same_v<AType, ecs::SphereColliderComponent> &&
                    std::is_same_v<BType, ecs::PlaneColliderComponent>
                ) {
                    collisionsStorage[static_cast<uint32_t>(CollisionType::SphereToPlane)].push_back({indexA, indexB});
                } else if constexpr (
                    std::is_same_v<AType, ecs::PlaneColliderComponent> &&
                    std::is_same_v<BType, ecs::SphereColliderComponent>
                ) {
                    collisionsStorage[static_cast<uint32_t>(CollisionType::PlaneToSphere)].push_back({indexA, indexB});
                }
            },
            shapeA,
            shapeB
        );
    };
    std::fill(_dynamicToStaticCollisions.begin(), _dynamicToStaticCollisions.end(), CollisionBucket());
    for (size_t i = 0; i < _dynamicAABBs.size(); ++i) {
        for (size_t j = 0; j < _staticAABBs.size(); ++j) {
            if (_dynamicAABBs[i].aabb.intersects(_staticAABBs[j].aabb)) {
                auto& colliderShapeDynamic =
                    bodyStorage.dynamicBodyColliders[_dynamicAABBs[i].colliderIndex].shapeCollider;
                auto& colliderShapeStatic =
                    bodyStorage.staticBodyColliders[_staticAABBs[j].colliderIndex].shapeCollider;
                processCollision(
                    _dynamicToStaticCollisions,
                    colliderShapeDynamic,
                    colliderShapeStatic,
                    static_cast<uint32_t>(i),
                    static_cast<uint32_t>(j)
                );
            }
        }
    }

    // TODO: fix this, remove O(n2)
    std::fill(_dynamicToDynamicCollisions.begin(), _dynamicToDynamicCollisions.end(), CollisionBucket());
    for (size_t i = 0; i < _dynamicAABBs.size(); ++i) {
        for (size_t j = i + 1; j < _dynamicAABBs.size(); ++j) {
            if (_dynamicAABBs[i].aabb.intersects(_dynamicAABBs[j].aabb)) {
                const auto& colliderA = bodyStorage.dynamicBodyColliders[_dynamicAABBs[i].colliderIndex];
                const auto& colliderB = bodyStorage.dynamicBodyColliders[_dynamicAABBs[j].colliderIndex];
                if (colliderA.bodyIndex == colliderB.bodyIndex) [[unlikely]] {
                    continue;
                }
                processCollision(
                    _dynamicToDynamicCollisions,
                    colliderA.shapeCollider,
                    colliderB.shapeCollider,
                    static_cast<uint32_t>(i),
                    static_cast<uint32_t>(j)
                );
            }
        }
    }
}