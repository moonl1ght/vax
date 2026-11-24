#pragma once

#include "textureConfig.h"
#include <nlohmann/json.hpp>
#include <string>

namespace vax::scene {
struct MaterialConfig final {
    std::string name;
    TextureConfig textureConfig;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MaterialConfig, name, textureConfig)
} // namespace vax::scene
