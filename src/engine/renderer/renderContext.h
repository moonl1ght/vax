#pragma once

#include "shaderResourceHandle.h"
#include <cstdint>

namespace vax::engine {
struct RenderCallContext {
    uint32_t currentFrame;
    ShaderResourceIndex shadowMapIndex = NullShaderResourceIndex;
};
} // namespace vax::engine