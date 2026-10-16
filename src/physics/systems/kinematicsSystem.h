#pragma once

namespace vax::physics {

class KinematicsSystem {
  public:
    KinematicsSystem() = default;
    ~KinematicsSystem() = default;

    void step();
};

} // namespace vax::physics