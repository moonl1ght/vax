#pragma once

#include "buffer.h"
#include "descriptorRegistry.h"
#include "device.h"
#include "logger.h"
#include "pbr.h"
#include "resourceUtils.h"
#include <memory>
#include <optional>
#include <vector>

namespace vax::rhi {
using MaterialBuffer = Buffer<PBRMaterial>;
using MaterialBufferResource = std::pair<StructuredBufferShaderHandle, MaterialBuffer>;

class MaterialManager final {
  public:
    explicit MaterialManager(const Device& device, rhi::DescriptorRegistry& descriptorRegistry)
        : _device(device)
        , _descriptorRegistry(descriptorRegistry) {};

    ~MaterialManager() { cleanup(); }

    MaterialManager(const MaterialManager& other) = delete;
    MaterialManager(MaterialManager&& other) noexcept = delete;
    MaterialManager& operator=(const MaterialManager& other) = delete;
    MaterialManager& operator=(MaterialManager&& other) noexcept = delete;

    bool load() noexcept;

    void cleanup();

    MaterialId insert(PBRMaterial material);

    std::vector<MaterialId> insertMaterials(std::vector<PBRMaterial> materials);

    std::optional<PBRMaterial> find(MaterialId id);

    void deleteMaterial(MaterialId id);

    const MaterialBufferResource& materialBuffer() const { return *_buffer; }

  private:
    dev::Logger _logger = dev::Logger("MaterialManager");
    std::reference_wrapper<const Device> _device;
    std::reference_wrapper<rhi::DescriptorRegistry> _descriptorRegistry;
    // TODO: change to vector of buffers to handle material overflow
    std::unique_ptr<MaterialBufferResource> _buffer = nullptr;
    std::vector<PBRMaterial> _materials;
    std::vector<MaterialId> _materialsToDelete;

    bool _updateBuffer(MaterialId id, PBRMaterial material);
};
} // namespace vax::rhi