#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace vax::engine {

struct CameraDescriptor final {
  public:
    CameraDescriptor(const std::string& name) : _name(name) {}
    ~CameraDescriptor() = default;

  private:
    std::string _name;
};

} // namespace vax::engine