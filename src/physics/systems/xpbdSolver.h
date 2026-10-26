#pragma once

namespace vax::physics {

class XPBDSolver final {
  public:
    XPBDSolver() = default;
    ~XPBDSolver() = default;

    void solve();
};

} // namespace vax::physics