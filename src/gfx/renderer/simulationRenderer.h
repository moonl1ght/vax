#pragma once

namespace vax::gfx {

class SimulationRenderer {
  public:
    explicit SimulationRenderer() noexcept = default;
    ~SimulationRenderer() = default;

    SimulationRenderer(const SimulationRenderer&) = delete;
    SimulationRenderer(SimulationRenderer&&) noexcept = delete;
    SimulationRenderer& operator=(const SimulationRenderer&) = delete;
    SimulationRenderer& operator=(SimulationRenderer&&) noexcept = delete;
};

} // namespace vax::gfx