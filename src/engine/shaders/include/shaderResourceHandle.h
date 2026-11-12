#ifndef shaderResourceHandle_h
#define shaderResourceHandle_h

#ifdef __cplusplus

#include <cstdint>

typedef uint32_t ShaderResourceIndex;

struct SampledImageTag {};
struct StorageImageTag {};
struct StructuredBufferTag {};
struct ConstantBufferTag {};
struct SamplerTag {};

template <typename Tag> struct ShaderResourceShaderHandle {
    ShaderResourceIndex index = 0;
    ShaderResourceIndex _padding = 0;
};

using SampledImageShaderHandle = ShaderResourceShaderHandle<SampledImageTag>;
using StorageImageShaderHandle = ShaderResourceShaderHandle<StorageImageTag>;
using StructuredBufferShaderHandle = ShaderResourceShaderHandle<StructuredBufferTag>;
using ConstantBufferShaderHandle = ShaderResourceShaderHandle<ConstantBufferTag>;
using SamplerShaderHandle = ShaderResourceShaderHandle<SamplerTag>;

#define SAMPLED_IMAGE_HANDLE(T) SampledImageShaderHandle
#define STORAGE_IMAGE_HANDLE(T) StorageImageShaderHandle
#define STRUCTURED_BUFFER_HANDLE(T) StructuredBufferShaderHandle
#define CONSTANT_BUFFER_HANDLE(T) ConstantBufferShaderHandle
#define SAMPLER_HANDLE(T) SamplerShaderHandle

#else // Slang

#define SAMPLED_IMAGE_HANDLE(T) T.Handle
#define STORAGE_IMAGE_HANDLE(T) T.Handle
#define STRUCTURED_BUFFER_HANDLE(T) T.Handle
#define CONSTANT_BUFFER_HANDLE(T) T.Handle
#define SAMPLER_HANDLE(T) T.Handle

#endif

#endif
