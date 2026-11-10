#include "materialManager.h"
#include <cstring>
#include <numeric>

using namespace vax::vk;

void MaterialManager::cleanup() {
    _materials.clear();
    _buffer->cleanup();
}

bool MaterialManager::setup() {
    VkDeviceSize bufferSize = sizeof(PBRMaterial) * vax::vk::MAX_MATERIALS;
    auto allocation = MaterialBuffer::allocate(
        _device.get(),
        "global_material_buffer",
        bufferSize,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
    );
    if (!allocation.has_value())
        return false;
    _buffer = std::make_unique<MaterialBuffer>(std::move(*allocation));
    _buffer->map();
    _materials.reserve(vax::vk::MAX_MATERIALS);
    _materialsToDelete.reserve(vax::vk::MAX_MATERIALS);
    return true;
}

bool MaterialManager::_updateBuffer(MaterialId id, PBRMaterial material) {
    if (!_buffer)
        return false;
    auto mappedMemory = _buffer->mappedMemory();
    if (!mappedMemory.has_value())
        return false;
    PBRMaterial* materialPtr = static_cast<PBRMaterial*>(*mappedMemory);
    materialPtr[id] = material;
    return true;
}

std::vector<MaterialId> MaterialManager::insertMaterials(std::vector<PBRMaterial> materials) {
    if (_materials.size() + materials.size() >= vax::vk::MAX_MATERIALS)
        return {};
    if (materials.empty())
        return {};
    if (!_buffer)
        return {};
    auto mappedMemory = _buffer->mappedMemory();
    if (!mappedMemory.has_value())
        return {};
    PBRMaterial* materialPtr = mappedMemory.value();
    memcpy(materialPtr + _materials.size(), materials.data(), materials.size() * sizeof(PBRMaterial));
    std::vector<MaterialId> ids(materials.size());
    std::iota(ids.begin(), ids.end(), _materials.size());
    _materials.insert(_materials.end(), materials.begin(), materials.end());
    return ids;
}

MaterialId MaterialManager::insert(PBRMaterial material) {
    if (!_materialsToDelete.empty()) {
        auto id = _materialsToDelete.back();
        if (!_updateBuffer(id, material))
            return NullMaterialId;
        _materials[id] = material;
        _materialsToDelete.pop_back();
        return id;
    }
    if (_materials.size() >= vax::vk::MAX_MATERIALS)
        return NullMaterialId;
    if (!_updateBuffer(_materials.size(), material))
        return NullMaterialId;
    _materials.push_back(material);
    return _materials.size() - 1;
}

std::optional<PBRMaterial> MaterialManager::find(MaterialId id) {
    if (id >= _materials.size())
        return std::nullopt;
    return _materials[id];
}

void MaterialManager::deleteMaterial(MaterialId id) {
    if (id >= _materials.size())
        return;
    _materialsToDelete.push_back(id);
    return;
}
