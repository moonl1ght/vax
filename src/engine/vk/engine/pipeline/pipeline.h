#pragma once

#include "device.h"
#include "luna.h"

namespace vax::vk {
enum class PipelineLayoutName : uint32_t {
    BACKGROUND = 0,
    BASE = 1,
    FINAL_BLEND = 2,
    AGENT_CAMERA_FB = 3,
    UNKNOWN = 0xFFFFFFFF,
};

enum class PipelineName : uint32_t {
    BACKGROUND = 0,
    PBR = 1,
    BASE = 2,
    FINAL_BLEND = 3,
    AGENT_CAMERA = 4,
    AGENT_CAMERA_FB = 5,
    SHADOW = 6,
    UNKNOWN = 0xFFFFFFFF,
};

enum class PipelineType { RENDER, COMPUTE, UNKNOWN };

class Pipeline final {
  public:
    static std::string pipelineNameToString(PipelineName name) {
        switch (name) {
        case PipelineName::BACKGROUND:
            return "background_pipeline";
        case PipelineName::PBR:
            return "pbr_pipeline";
        case PipelineName::BASE:
            return "base_pipeline";
        case PipelineName::FINAL_BLEND:
            return "final_blend_pipeline";
        case PipelineName:: AGENT_CAMERA:
            return "agent_camera_pipeline";
        case PipelineName::AGENT_CAMERA_FB:
            return "agent_camera_fb_pipeline";
        case PipelineName::UNKNOWN:
            return "unknown_pipeline";
        default:
            return "unknown_pipeline";
        }
    };

    static std::string pipelineLayoutNameToString(PipelineLayoutName name) {
        switch (name) {
        case PipelineLayoutName::BACKGROUND:
            return "background_pipeline_layout";
        case PipelineLayoutName::BASE:
            return "base_pipeline_layout";
        case PipelineLayoutName::FINAL_BLEND:
            return "final_blend_pipeline_layout";
        case PipelineLayoutName::AGENT_CAMERA_FB:
            return "agent_camera_fb_pipeline_layout";
        case PipelineLayoutName::UNKNOWN:
            return "unknown_pipeline_layout";
        default:
            return "unknown_pipeline_layout";
        }
    };

    std::string name = "";
    PipelineType pipelineType = PipelineType::UNKNOWN;
    VkPipelineLayout vkPipelineLayout = VK_NULL_HANDLE;
    VkPipeline vkPipeline = VK_NULL_HANDLE;

    Pipeline(const Pipeline& other) = delete;
    Pipeline& operator=(const Pipeline& other) = delete;

    Pipeline(Pipeline&& other) noexcept
        : _device(other._device)
        , name(other.name)
        , pipelineType(other.pipelineType)
        , vkPipelineLayout(other.vkPipelineLayout)
        , vkPipeline(other.vkPipeline) {
        other.pipelineType = PipelineType::UNKNOWN;
        other.vkPipelineLayout = VK_NULL_HANDLE;
        other.vkPipeline = VK_NULL_HANDLE;
        other.name.clear();
    };

    Pipeline& operator=(Pipeline&& other) noexcept {
        if (this != &other) {
            if (vkPipelineLayout != VK_NULL_HANDLE) {
                vkDestroyPipelineLayout(_device.get().vkDevice, vkPipelineLayout, nullptr);
            }
            if (vkPipeline != VK_NULL_HANDLE) {
                vkDestroyPipeline(_device.get().vkDevice, vkPipeline, nullptr);
            }
            _device = other._device;
            pipelineType = other.pipelineType;
            vkPipelineLayout = other.vkPipelineLayout;
            vkPipeline = other.vkPipeline;
            name = other.name;
            other.pipelineType = PipelineType::UNKNOWN;
            other.vkPipelineLayout = VK_NULL_HANDLE;
            other.vkPipeline = VK_NULL_HANDLE;
            other.name.clear();
        }
        return *this;
    };

    explicit Pipeline(
        const vax::vk::Device& device,
        std::string name,
        PipelineType pipelineType,
        VkPipelineLayout pipelineLayout,
        VkPipeline pipeline
    ) noexcept
        : _device(device)
        , name(name)
        , pipelineType(pipelineType)
        , vkPipelineLayout(pipelineLayout)
        , vkPipeline(pipeline) {};

    ~Pipeline() { vkDestroyPipeline(_device.get().vkDevice, vkPipeline, nullptr); };

  private:
    vax::Logger _logger = vax::Logger("Pipeline");
    std::reference_wrapper<const vax::vk::Device> _device;
};
} // namespace vax::vk
