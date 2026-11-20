#pragma once

#include "rlUtils.h"

namespace vax::rl {
template <typename Derived, typename State, typename Action> class Agent {
  public:
    Agent() {};
    ~Agent() = default;

    Agent(const Agent& other) = delete;
    Agent& operator=(const Agent& other) = delete;
    Agent(Agent&& other) noexcept = delete;
    Agent& operator=(Agent&& other) noexcept = delete;

    Action chooseAction(const State& state) { return static_cast<Derived*>(this)->chooseActionImpl(state); }

    void update(const State& state, Action action, double reward, const State& nextState, bool done) {
        static_cast<Derived*>(this)->updateImpl(state, action, reward, nextState, done);
    }

    void setEvalMode(vax::rl::EvalMode evalMode) { static_cast<Derived*>(this)->setEvalModeImpl(evalMode); }
};
} // namespace vax::rl