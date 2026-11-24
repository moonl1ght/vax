#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace vax::scene {

struct CameraConfig final {
    std::string name;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CameraConfig, name)

} // namespace vax::scene