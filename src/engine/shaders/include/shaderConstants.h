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

#endif