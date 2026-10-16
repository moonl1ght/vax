#pragma once

namespace vax::physics {

class CollisionDetector {
  public:
    CollisionDetector() = default;
    ~CollisionDetector() = default;

    void detectCollisions();
};

} // namespace vax::physics