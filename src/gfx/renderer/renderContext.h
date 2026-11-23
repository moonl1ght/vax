#pragma once

#include "shaderResourceHandle.h"
#include <cstdint>

namespace vax::gfx {
struct RenderCallContext {
    uint32_t currentFrame;
    ShaderResourceIndex shadowMapIndex = NullShaderResourceIndex;
};
} // namespace vax::gfx