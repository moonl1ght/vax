#pragma once

#include "logger.h"
#include "renderDestination.h"
#include "renderPassDescriptor.h"
#include <unordered_map>

namespace vax::gfx {

class RenderTarget final {
  public:
    RenderTarget(
        std::unordered_map<std::string, std::unique_ptr<vax::rhi::RenderPassDescriptor>> renderPassDescriptors,
        std::unordered_map<std::string, std::unique_ptr<vax::rhi::RenderDestination>> renderDestinations
    )
        : _renderPassDescriptors(std::move(renderPassDescriptors))
        , _renderDestinations(std::move(renderDestinations)) {}

    ~RenderTarget() = default;

    vax::rhi::RenderPassDescriptor* getRenderPassDescriptor(std::string name) const {
        if (_renderPassDescriptors.contains(name)) {
            return _renderPassDescriptors.at(name).get();
        }
        return nullptr;
    }

    vax::rhi::RenderDestination* getRenderDestination(std::string name) const {
        if (_renderDestinations.contains(name)) {
            return _renderDestinations.at(name).get();
        }
        return nullptr;
    }

  private:
    dev::Logger _logger = dev::Logger("renderTarget");

    std::unordered_map<std::string, std::unique_ptr<vax::rhi::RenderPassDescriptor>> _renderPassDescriptors;
    std::unordered_map<std::string, std::unique_ptr<vax::rhi::RenderDestination>> _renderDestinations;
};
} // namespace vax::gfx