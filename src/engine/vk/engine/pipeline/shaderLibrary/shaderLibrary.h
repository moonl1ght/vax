#pragma once

#include <slang.h>
#include <slang-com-ptr.h>

namespace vax::vk {

class ShaderLibrary final {
  public:
    ShaderLibrary() = default;
    ~ShaderLibrary() = default;

    ShaderLibrary(const ShaderLibrary&) = delete;
    ShaderLibrary(ShaderLibrary&&) = delete;
    ShaderLibrary& operator=(const ShaderLibrary&) = delete;
    ShaderLibrary& operator=(ShaderLibrary&&) = delete;

  private:
  Slang::ComPtr<slang::ISession> session;
};

} // namespace vax::vk