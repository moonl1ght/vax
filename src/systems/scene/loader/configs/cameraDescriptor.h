#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace vax::gfx {

struct CameraDescriptor final {
    std::string name;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CameraDescriptor, name)

} // namespace vax::gfx