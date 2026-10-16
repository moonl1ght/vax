#pragma once

namespace vax::physics {

class XPBDSolver {
  public:
    XPBDSolver() = default;
    ~XPBDSolver() = default;

    void solve();
};

} // namespace vax::physics