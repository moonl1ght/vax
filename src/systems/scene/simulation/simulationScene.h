#pragma once

namespace vax::scene {

class SimulationScene final {
  public:
    explicit SimulationScene() {};
    ~SimulationScene() = default;

    SimulationScene(const SimulationScene&) = delete;
    SimulationScene(SimulationScene&&) noexcept = delete;
    SimulationScene& operator=(const SimulationScene&) = delete;
    SimulationScene& operator=(SimulationScene&&) noexcept = delete;
};

} // namespace vax::scene