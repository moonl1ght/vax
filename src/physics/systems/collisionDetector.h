#pragma once

#include "physicsComponents.h"

#include <vector>

namespace vax::physics {

class CollisionDetector final {
  public:
    struct CollisionPair final {
        uint32_t first;
        uint32_t second;
    };

    CollisionDetector() = default;
    ~CollisionDetector() = default;

    CollisionDetector(const CollisionDetector& other) = delete;
    CollisionDetector& operator=(const CollisionDetector& other) = delete;
    CollisionDetector(CollisionDetector&& other) noexcept = delete;
    CollisionDetector& operator=(CollisionDetector&& other) noexcept = delete;

    void buildDynamicAABB(const BodyStorage& bodyStorage, float dt) noexcept;

    void buildStaticAABB(const BodyStorage& bodyStorage) noexcept;

    void findCollisionPairs() noexcept;

    void detectCollisions() noexcept;

  private:
    std::vector<AABB> _dynamicAABBs;
    std::vector<AABB> _staticAABBs;
    std::vector<CollisionPair> _staticToDynamicCollisionPairs;
    std::vector<CollisionPair> _dynamicToDynamicCollisionPairs;
};

} // namespace vax::physics