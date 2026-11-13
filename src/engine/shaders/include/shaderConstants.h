#ifndef shaderConstants_h
#define shaderConstants_h

static constexpr uint MAX_UINT = 60000;

enum {
    BINDLESS_SET_INDEX = 0,
    UBO_SET_INDEX = 1,
};

enum {
    SAMPLED_IMAGE_BINDING_INDEX = 0,
    STORAGE_IMAGE_BINDING_INDEX = 1,
    SAMPLER_BINDING_INDEX = 2,
    STORAGE_BUFFER_BINDING_INDEX = 3,
};

enum {
    FRAME_UNIFORM_BUFFER_INDEX = 0,
    LIGHT_UNIFORM_BUFFER_INDEX = 1,
};

static const uint32_t NO_TEXTURE_FLAG = 0xFFFFFFFF;
static const uint32_t NO_MATERIAL_INDEX = 0xFFFFFFFF;
static const uint32_t NO_ENVIRONMENT_MAP_INDEX = 0xFFFFFFFF;
static const uint32_t NO_SAMPLER_INDEX = 0xFFFFFFFF;
static const uint32_t NO_SHADOW_MAP_INDEX = 0xFFFFFFFF;
static const uint32_t NO_ID = 0xFFFFFFFF;

static const uint32_t MAX_UBO_LIGHTS = 16;

#endif