#ifndef shaderResourceHandle_h
#define shaderResourceHandle_h

#ifdef __cplusplus

#include <cstdint>

typedef uint32_t ShaderResourceIndex;

struct SampledImageTag {};
struct StorageImageTag {};

template <typename Tag> struct ShaderResourceHandle {
    ShaderResourceIndex index = 0;
    ShaderResourceIndex _padding = 0;
};

using SampledHandle = ShaderResourceHandle<SampledImageTag>;
using StorageHandle = ShaderResourceHandle<StorageImageTag>;

#define SAMPLED_IMAGE_HANDLE(T) SampledHandle
#define STORAGE_IMAGE_HANDLE(T) StorageHandle

#else // Slang

#define SAMPLED_IMAGE_HANDLE(T) T.Handle
#define STORAGE_IMAGE_HANDLE(T) T.Handle

#endif

#endif
