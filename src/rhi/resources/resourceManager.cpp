#include "resourceManager.h"

using namespace vax::rhi;

void ResourceManager::_cleanup() {
    _bufferManager.fullCleanup();
    _meshManager.cleanup();
    _materialManager.cleanup();
    _ssboManager.cleanup();
    _logger.debug("ResourceManager cleanup completed");
}

bool ResourceManager::load(uint32_t maxInstances) noexcept {
    if (!_meshManager.load(1024))
        return false;
    if (!_materialManager.load())
        return false;
    if (!_ssboManager.setup(maxInstances))
        return false;
    return true;
}

bool ResourceManager::setup() noexcept {
    if (!_samplerManager.setup())
        return false;
    return true;
}

void ResourceManager::cleanupEditorSceneResources() noexcept {
    _meshManager.cleanup();
    _environmentMap.cleanup();
    _materialManager.cleanup();
    _ssboManager.cleanup();
}