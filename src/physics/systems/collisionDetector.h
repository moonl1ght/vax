#pragma once

namespace vax::physics {

class CollisionDetector final {
  public:
    CollisionDetector() = default;
    ~CollisionDetector() = default;

    void detectCollisions();
};

} // namespace vax::physics