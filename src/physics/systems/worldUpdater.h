#pragma once

namespace vax::physics {

class WorldUpdater {
  public:
    WorldUpdater() = default;
    ~WorldUpdater() = default;

    void update();
};

} // namespace vax::physics