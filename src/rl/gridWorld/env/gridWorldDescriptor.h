#pragma once

#include "drawableModel.h"
#include "prefabDescriptor.h"

namespace vax::rl {
struct GridWorldDrawableDescriptor final {
    std::vector<vax::engine::PrefabDescriptor> drawableDescriptors;
    vax::engine::PrefabDescriptor agentDrawableDescriptor;
};
} // namespace vax::rl