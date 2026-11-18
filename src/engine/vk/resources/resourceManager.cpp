#include "resourceManager.h"

using namespace vax::vk;

void ResourceManager::_cleanup() {
    _bufferManager.fullCleanup();
    _meshManager.fullCleanup();
    _textureManager.cleanup();
    _materialManager.cleanup();
    _ssboManager.cleanup();
    _logger.debug("ResourceManager cleanup completed");
}

bool ResourceManager::load(uint32_t maxInstances) noexcept {
    _meshManager.loadGlobalBuffers(1024);
    if (!_materialManager.setup())
        return false;
    if (!_ssboManager.setup(maxInstances))
        return false;
    return true;
}

bool ResourceManager::setup() noexcept {
    if (!_textureManager.setup())
        return false;
    return true;
}