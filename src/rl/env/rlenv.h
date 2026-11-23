#pragma once

#include <string>
#include "rlUtils.h"

namespace vax::rl {
struct StepResult final {
    double reward = 0.0;
    bool done = false;
    bool finishedWithError = false;
};

template <typename Derived, typename State, typename Action> class Environment {
  public:
    Environment() {};
    ~Environment() = default;

    Environment(const Environment& other) = delete;
    Environment& operator=(const Environment& other) = delete;
    Environment(Environment&& other) noexcept = delete;
    Environment& operator=(Environment&& other) noexcept = delete;

    State reset() { return static_cast<Derived*>(this)->resetImpl(); }

    State getState() const { return static_cast<const Derived*>(this)->getStateImpl(); }

    StepResult step(Action action) { return static_cast<Derived*>(this)->stepImpl(action); }

    const std::string& name() const { return static_cast<const Derived*>(this)->nameImpl(); }

    void setEvalMode(vax::rl::EvalMode evalMode) { static_cast<Derived*>(this)->setEvalModeImpl(evalMode); }
};
} // namespace vax::rl