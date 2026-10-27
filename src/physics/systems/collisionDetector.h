#pragma once

#include "physicsComponents.h"

#include <vector>

namespace vax::physics {

class CollisionDetector final {
  public:
    CollisionDetector() = default;
    ~CollisionDetector() = default;


    CollisionDetector(const CollisionDetector& other) = delete;
    CollisionDetector& operator=(const CollisionDetector& other) = delete;
    CollisionDetector(CollisionDetector&& other) noexcept = delete;
    CollisionDetector& operator=(CollisionDetector&& other) noexcept = delete;

    void buildAABB() noexcept;

    void detectCollisions() noexcept;

  private:
    std::vector<AABB> _dynamicAABBs;
    std::vector<AABB> _staticAABBs;
};

} // namespace vax::physics