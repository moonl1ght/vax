#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace vax::engine {

struct SceneDescriptor final {
  public:
    SceneDescriptor(const std::string& name) : _name(name) {}
    ~SceneDescriptor() = default;

  private:
    std::string _name;
    bool _shouldDrawGizmo;
};

} // namespace vax::engine