#include "shaderModule.h"
#include <fstream>

using namespace vax::vk;

std::optional<ShaderModule> ShaderModuleBuilder::build(const std::vector<uint32_t>& spirv) {
    VkShaderModuleCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = spirv.size() * sizeof(uint32_t),
        .pCode = spirv.data(),
    };
    VkShaderModule shaderModule;
    if (!VK_CHECK(vkCreateShaderModule(_device.get().vkDevice, &createInfo, nullptr, &shaderModule))) {
        _logger.error("Failed to build shader module!");
        return std::nullopt;
    }
    return ShaderModule(_device.get(), shaderModule);
}

std::optional<VkShaderModule> ShaderModuleBuilder::build(const std::string& filename) {
    auto code = _readFile(filename);

    if (code.empty()) {
        _logger.error("Failed to build shader code!");
        return std::nullopt;
    }
    VkShaderModuleCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = code.size(),
        .pCode = reinterpret_cast<const uint32_t*>(code.data()),
    };

    VkShaderModule shaderModule;
    if (!VK_CHECK(vkCreateShaderModule(_device.get().vkDevice, &createInfo, nullptr, &shaderModule))) {
        _logger.error("Failed to build shader module!");
        return std::nullopt;
    }

    return shaderModule;
}

std::vector<char> ShaderModuleBuilder::_readFile(const std::string& filename) const {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        _logger.error("failed to open file!");
        return std::vector<char>();
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> code(fileSize);

    file.seekg(0);
    file.read(code.data(), fileSize);

    file.close();
    return code;
}
