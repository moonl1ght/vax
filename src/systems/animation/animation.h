#pragma once

#include "frameTime.h"

namespace vax::animation {

template <typename Derived> class Animation {
  public:
    Animation() = default;
    ~Animation() = default;

    bool update(const vax::time::FrameTime& frameTime) { return static_cast<Derived*>(this)->updateImpl(frameTime); }
};

} // namespace vax::animation