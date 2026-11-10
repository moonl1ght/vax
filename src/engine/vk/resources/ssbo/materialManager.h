#pragma once

#include "buffer.h"
#include "device.h"
#include "logger.h"
#include <memory>
#include <optional>
#include <vector>
#include "resourceUtils.h"
#include "shaderUniforms.h"

namespace vax::vk {
using MaterialBuffer = Buffer<PBRMaterial>;

class MaterialManager final {
  public:
    explicit MaterialManager(const Device& device)
        : _device(device) {};

    ~MaterialManager() { cleanup(); }

    MaterialManager(const MaterialManager& other) = delete;
    MaterialManager(MaterialManager&& other) noexcept = delete;
    MaterialManager& operator=(const MaterialManager& other) = delete;
    MaterialManager& operator=(MaterialManager&& other) noexcept = delete;

    bool setup();

    void cleanup();

    MaterialId insert(PBRMaterial material);

    std::vector<MaterialId> insertMaterials(std::vector<PBRMaterial> materials);

    std::optional<PBRMaterial> find(MaterialId id);

    void deleteMaterial(MaterialId id);

    const MaterialBuffer& materialBuffer() const { return *_buffer; }

  private:
    vax::Logger _logger = vax::Logger("MaterialManager");
    std::reference_wrapper<const Device> _device;
    // TODO: change to vector of buffers to handle material overflow
    std::unique_ptr<MaterialBuffer> _buffer = nullptr;
    std::vector<PBRMaterial> _materials;
    std::vector<MaterialId> _materialsToDelete;

    bool _updateBuffer(MaterialId id, PBRMaterial material);
};
} // namespace vax::vk