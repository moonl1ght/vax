#pragma once

#include "frameTime.h"

namespace vax {

template <typename Derived> class Animation {
  public:
    Animation() = default;
    ~Animation() = default;

    bool update(const engine::FrameTime& frameTime) { return static_cast<Derived*>(this)->updateImpl(frameTime); }
};
} // namespace vax