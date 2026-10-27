#include "collisionDetector.h"

using namespace vax::physics;

void CollisionDetector::detectCollisions() noexcept {
    // TODO: Implement collision detector
}

void CollisionDetector::buildDynamicAABB(const BodyStorage& bodyStorage, float dt) noexcept {
    _dynamicAABBs.clear();
    _dynamicAABBs.reserve(bodyStorage.dynamicBodyColliders.size());
    for (size_t i = 0; i < bodyStorage.dynamicBodyColliders.size(); ++i) {
        auto& collider = bodyStorage.dynamicBodyColliders[i];
        const auto& body = bodyStorage.dynamicBodies[collider.bodyIndex];
        AABB aabb = AABB::getAABB(collider.shapeCollider, body.position, body.orientation).value_or(AABB());
        const glm::vec3 delta = body.velocity * dt;
        aabb.min += glm::min(delta, glm::vec3(0.0f));
        aabb.max += glm::max(delta, glm::vec3(0.0f));
        _dynamicAABBs.push_back(aabb);
    }
}

void CollisionDetector::buildStaticAABB(const BodyStorage& bodyStorage) noexcept {
    _staticAABBs.clear();
    _staticAABBs.reserve(bodyStorage.staticBodyColliders.size());
    for (size_t i = 0; i < bodyStorage.staticBodyColliders.size(); ++i) {
        auto& collider = bodyStorage.staticBodyColliders[i];
        const auto& body = bodyStorage.staticBodies[collider.bodyIndex];
        AABB aabb = AABB::getAABB(collider.shapeCollider, body.position, body.orientation).value_or(AABB());
        _staticAABBs.push_back(aabb);
    }
}

void CollisionDetector::findCollisionPairs() noexcept {
    _staticToDynamicCollisionPairs.clear();
    _staticToDynamicCollisionPairs.reserve(_dynamicAABBs.size() * _staticAABBs.size());
    for (size_t i = 0; i < _dynamicAABBs.size(); ++i) {
        for (size_t j = 0; j < _staticAABBs.size(); ++j) {
            if (_dynamicAABBs[i].intersects(_staticAABBs[j])) {
                _staticToDynamicCollisionPairs.push_back({static_cast<uint32_t>(i), static_cast<uint32_t>(j)});
            }
        }
    }

    // TODO: fix this, remove O(n2)
    for (size_t i = 0; i < _dynamicAABBs.size(); ++i) {
        for (size_t j = 0; j < _dynamicAABBs.size(); ++j) {
            if (i == j) [[unlikely]] {
                continue;
            }
            if (_dynamicAABBs[i].intersects(_dynamicAABBs[j])) {
                _dynamicToDynamicCollisionPairs.push_back({static_cast<uint32_t>(i), static_cast<uint32_t>(j)});
            }
        }
    }
}

std::optional<ContactConstraint> sphereToSphere(
    uint32_t indexFirst,
    const glm::vec3& positionFirst,
    float radiusFirst,
    const DynamicBody& bodyFirst,
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

    const glm::vec3 normal = length > 1e-6f ? delta / length : glm::vec3(0.0f, 1.0f, 0.0f);

    const auto& firstRigidBodyComponent = bodyFirst.rigidBodyComponent;
    return ContactConstraint{
        .first = indexFirst,
        .second = indexSecond,
        .secondIsStatic = isSecondStatic,
        .normalFromFirstToSecond = normal,
        .localPointFirst = normal * radiusFirst,
        .localPointSecond = -normal * radiusSecond,
        .penetrationDepth = radiusSum - length,
        .staticFriction = 0.5f * (firstRigidBodyComponent.staticFriction + staticFrictionSecond),
        .dynamicFriction = 0.5f * (firstRigidBodyComponent.dynamicFriction + dynamicFrictionSecond),
        .restitution = std::max(firstRigidBodyComponent.restitution, restitutionSecond),
        .normalVelocityBefore = 0.0f,
    };
}