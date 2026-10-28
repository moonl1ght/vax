#pragma once

#include "physicsComponents.h"

#include <array>
#include <vector>

namespace vax::physics {

class CollisionDetector final {
  public:
    constexpr static float EPSILON = 1e-6f;

    struct ColliderHandler final {
        AABB aabb;
        uint32_t colliderIndex;
    };

    struct CollisionPair final {
        uint32_t first;
        uint32_t second;
    };

    enum class CollisionType : uint32_t {
        SphereToSphere = 0,
        SphereToPlane = 1,
        PlaneToSphere = 2,
    };

    constexpr static size_t NUM_COLLISION_TYPES = 3;

    using CollisionBucket = std::vector<CollisionPair>;

    CollisionDetector() = default;
    ~CollisionDetector() = default;

    CollisionDetector(const CollisionDetector& other) = delete;
    CollisionDetector& operator=(const CollisionDetector& other) = delete;
    CollisionDetector(CollisionDetector&& other) noexcept = delete;
    CollisionDetector& operator=(CollisionDetector&& other) noexcept = delete;

    // broad phase
    void buildDynamicAABB(const BodyStorage& bodyStorage, float dt) noexcept;

    void buildStaticAABB(const BodyStorage& bodyStorage) noexcept;

    void findCollisionPairs(const BodyStorage& bodyStorage) noexcept;

    // narrow phase
    void detectCollisions(const BodyStorage& bodyStorage) noexcept;

    const std::vector<ContactConstraint>& getContactConstraints() const noexcept { return _contactConstraints; }

  private:
    std::vector<ColliderHandler> _dynamicAABBs;
    std::vector<ColliderHandler> _staticAABBs;
    std::array<CollisionBucket, NUM_COLLISION_TYPES> _dynamicToStaticCollisions;
    std::array<CollisionBucket, NUM_COLLISION_TYPES> _dynamicToDynamicCollisions;
    std::vector<ContactConstraint> _contactConstraints;
};

} // namespace vax::physics