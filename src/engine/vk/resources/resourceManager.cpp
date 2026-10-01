#include "resourceManager.h"

using namespace vax::vk;

void ResourceManager::_cleanup() {
    _bufferManager.fullCleanup();
    _meshManager.fullCleanup();
    _textureManager.fullCleanup();
    _materialManager.cleanup();
    _ssboManager.cleanup();
    _logger.debug("ResourceManager cleanup complete");
}

bool ResourceManager::setup(uint32_t maxInstances) {
    _meshManager.loadGlobalBuffers(1024);
    if (!_materialManager.setup())
        return false;
    if (!_textureManager.setup())
        return false;
    if (!_ssboManager.setup(maxInstances))
        return false;
    return true;
}