#pragma once

#include "textureDescriptor.h"
#include <nlohmann/json.hpp>
#include <string>

namespace vax::engine {
struct MaterialDescriptor final {
    std::string name;
    TextureDescriptor textureDescriptor;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MaterialDescriptor, name, textureDescriptor)
} // namespace vax::engine
