#pragma once

#include "luna.h"
#include "shaderUniforms.h"
#include "vkUtils.h"
#include <cstdint>

namespace vax::vk {
enum class CommonDescriptorSetName {
    GLOBAL,
    PER_FRAME,
    AGENT_CAMERA,
    PER_DRAW,

    MAIN_FB,
    MAIN_FB_INPUT_MASK_0,
    MAIN_FB_INPUT_MASK_1,
    AGENT_CAMERA_FB,

    JFA_INIT,
    JFA_MAIN_0,
    JFA_MAIN_1,
};

enum class DescriptorSetLayoutName {
    GLOBAL,
    PER_FRAME,
    FINAL_BLEND,
    FINAL_BLEND_SIMPLE,
    SINGLE_STORAGE_IMAGE,
    JFA,
    PER_DRAW,
};

struct DescriptorSetInfo final {
    CommonDescriptorSetName name;
    DescriptorSetLayoutName layoutName;
    bool isUABPool;
};

struct DescriptorLayoutResourceInfo final {
    uint32_t binding;
    VkDescriptorType type;
    VkShaderStageFlags stageFlags;
    uint32_t descriptorCount;
};

constexpr std::vector<DescriptorSetInfo> allDescriptorSets() {
    return {
        {CommonDescriptorSetName::GLOBAL, DescriptorSetLayoutName::GLOBAL, true},
        {CommonDescriptorSetName::PER_FRAME, DescriptorSetLayoutName::PER_FRAME, false},
        {CommonDescriptorSetName::AGENT_CAMERA, DescriptorSetLayoutName::PER_FRAME, false},
        {CommonDescriptorSetName::PER_DRAW, DescriptorSetLayoutName::PER_DRAW, false},

        {CommonDescriptorSetName::MAIN_FB, DescriptorSetLayoutName::FINAL_BLEND, false},
        {CommonDescriptorSetName::MAIN_FB_INPUT_MASK_0, DescriptorSetLayoutName::SINGLE_STORAGE_IMAGE, false},
        {CommonDescriptorSetName::MAIN_FB_INPUT_MASK_1, DescriptorSetLayoutName::SINGLE_STORAGE_IMAGE, false},
        {CommonDescriptorSetName::AGENT_CAMERA_FB, DescriptorSetLayoutName::FINAL_BLEND_SIMPLE, false},

        {CommonDescriptorSetName::JFA_INIT, DescriptorSetLayoutName::JFA, false},
        {CommonDescriptorSetName::JFA_MAIN_0, DescriptorSetLayoutName::JFA, false},
        {CommonDescriptorSetName::JFA_MAIN_1, DescriptorSetLayoutName::JFA, false},
    };
}

constexpr std::string_view getSetLayoutName(const DescriptorSetLayoutName& setLayoutName) {
    switch (setLayoutName) {
    case DescriptorSetLayoutName::GLOBAL:
        return "global";
    case DescriptorSetLayoutName::PER_FRAME:
        return "per_frame";
    case DescriptorSetLayoutName::FINAL_BLEND:
        return "final_blend";
    case DescriptorSetLayoutName::FINAL_BLEND_SIMPLE:
        return "final_blend_simple";
    case DescriptorSetLayoutName::SINGLE_STORAGE_IMAGE:
        return "single_storage_image";
    case DescriptorSetLayoutName::PER_DRAW:
        return "per_draw";
    case DescriptorSetLayoutName::JFA:
        return "jfa";
    default:
        return "unknown_descriptor_set_layout";
    }
}

constexpr std::string_view getCommonDescriptorSetName(const CommonDescriptorSetName& commonDescriptorSetName) {
    switch (commonDescriptorSetName) {
    case CommonDescriptorSetName::GLOBAL:
        return "global";
    case CommonDescriptorSetName::PER_FRAME:
        return "per_frame";
    case CommonDescriptorSetName::AGENT_CAMERA:
        return "agent_camera";
    case CommonDescriptorSetName::PER_DRAW:
        return "per_draw";
    case CommonDescriptorSetName::MAIN_FB:
        return "main_fb";
    case CommonDescriptorSetName::MAIN_FB_INPUT_MASK_0:
        return "main_fb_input_mask_0";
    case CommonDescriptorSetName::MAIN_FB_INPUT_MASK_1:
        return "main_fb_input_mask_1";
    case CommonDescriptorSetName::AGENT_CAMERA_FB:
        return "agent_camera_fb";
    case CommonDescriptorSetName::JFA_INIT:
        return "jfa_init";
    case CommonDescriptorSetName::JFA_MAIN_0:
        return "jfa_main_0";
    case CommonDescriptorSetName::JFA_MAIN_1:
        return "jfa_main_1";
    }
}

constexpr std::vector<DescriptorLayoutResourceInfo>
getDescriptorLayoutResources(const DescriptorSetLayoutName& setLayoutName) {
    switch (setLayoutName) {
    case DescriptorSetLayoutName::GLOBAL:
        return {
            {
            .binding = GlobalDescriptorSetResourceIndex::GLOBAL_MATERIAL_BUFFER_INDEX,
            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .descriptorCount = 1,
            },
            {
            .binding = GlobalDescriptorSetResourceIndex::GLOBAL_ENVIRONMENT_MAP_BUFFER_INDEX,
            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .descriptorCount = 1,
            },
            {
            .binding = GlobalDescriptorSetResourceIndex::GLOBAL_SAMPLER_INDEX,
            .type = VK_DESCRIPTOR_TYPE_SAMPLER,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .descriptorCount = vax::vk::MAX_GLOBAL_SAMPLERS,
            },
            {
            .binding = GlobalDescriptorSetResourceIndex::GLOBAL_TEXTURE_INDEX,
            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .descriptorCount = MAX_TEXTURES + MAX_CUBE_MAP_TEXTURES,
            },
            {
            .binding = GlobalDescriptorSetResourceIndex::GLOBAL_SHADOW_TEXTURE_INDEX,
            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .descriptorCount = MAX_SHADOW_TEXTURES,
            },
        };
    case DescriptorSetLayoutName::PER_FRAME:
        return {
            {
            .binding = PerFrameDescriptorSetResourceIndex::FRAME_UNIFORM_BUFFER_INDEX,
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            .descriptorCount = 1,
            },
            {
            .binding = PerFrameDescriptorSetResourceIndex::FRAME_LIGHT_BUFFER_INDEX,
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .descriptorCount = 1,
            },
            {
            .binding = PerFrameDescriptorSetResourceIndex::FRAME_INSTANCE_BUFFER_INDEX,
            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            .descriptorCount = 1,
            },
        };
    case DescriptorSetLayoutName::FINAL_BLEND:
        return {
            {
            .binding = 0,
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .descriptorCount = 1,
            },
            {
            .binding = 1,
            .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .descriptorCount = 1,
            },
            {
            .binding = 2,
            .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .descriptorCount = 1,
            },
        };
    case DescriptorSetLayoutName::FINAL_BLEND_SIMPLE:
        return {
            {
            .binding = 0,
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .descriptorCount = 1,
            },
        };
    case DescriptorSetLayoutName::SINGLE_STORAGE_IMAGE:
        return {
            {
            .binding = 0,
            .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .descriptorCount = 1,
            },
        };
    case DescriptorSetLayoutName::PER_DRAW:
        return {
            {
            .binding = 0,
            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            .descriptorCount = 1,
            },
        };
    case DescriptorSetLayoutName::JFA: {
        return {
            {
            .binding = 0,
            .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
            .descriptorCount = 1,
            },
            {
            .binding = 1,
            .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
            .descriptorCount = 1,
            },
            {
            .binding = 2,
            .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
            .descriptorCount = 1,
            },
        };
    }
    default:
        return {};
    }
}

constexpr std::vector<VkDescriptorPoolSize>
getDescriptorLayoutPoolRequirements(const DescriptorSetLayoutName& setLayoutName) {
    switch (setLayoutName) {
    case DescriptorSetLayoutName::GLOBAL:
        return {
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MAX_TEXTURES + MAX_CUBE_MAP_TEXTURES + MAX_SHADOW_TEXTURES},
            {VK_DESCRIPTOR_TYPE_SAMPLER, vax::vk::MAX_GLOBAL_SAMPLERS},
        };
    case DescriptorSetLayoutName::PER_FRAME:
        return {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1},
        };
    case DescriptorSetLayoutName::FINAL_BLEND:
        return {
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2},
        };
    case DescriptorSetLayoutName::FINAL_BLEND_SIMPLE:
        return {
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
        };
    case DescriptorSetLayoutName::SINGLE_STORAGE_IMAGE:
        return {
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1},
        };
    case DescriptorSetLayoutName::PER_DRAW:
        return {
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1},
        };
    case DescriptorSetLayoutName::JFA:
        return {
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3},
        };
    default:
        return {};
    }
}
} // namespace vax::vk