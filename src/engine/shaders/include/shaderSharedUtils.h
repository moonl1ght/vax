#ifndef shaderSharedUtils_h
#define shaderSharedUtils_h

#ifdef __cplusplus

#include <glm/glm.hpp>

using mat4 = glm::mat4;
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;

inline uint32_t packRGBA(vec4 color) {
    uint8_t r = (uint8_t)(color.r * 255.0f);
    uint8_t g = (uint8_t)(color.g * 255.0f);
    uint8_t b = (uint8_t)(color.b * 255.0f);
    uint8_t a = (uint8_t)(color.a * 255.0f);
    return (a << 24) | (b << 16) | (g << 8) | r;
}

inline vec4 unpackRGBA(uint32_t rgba) {
    float r = (float)((rgba >> 0) & 0xFF);
    float g = (float)((rgba >> 8) & 0xFF);
    float b = (float)((rgba >> 16) & 0xFF);
    float a = (float)((rgba >> 24) & 0xFF);
    return vec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

#else // Slang

typedef float4x4 mat4;
typedef float2 vec2;
typedef float3 vec3;
typedef float4 vec4;

uint packRGBA(float4 color) {
    uint r = uint(color.r * 255.0);
    uint g = uint(color.g * 255.0);
    uint b = uint(color.b * 255.0);
    uint a = uint(color.a * 255.0);
    return (a << 24) | (b << 16) | (g << 8) | r;
}

float4 unpackRGBA(uint rgba) {
    float r = float((rgba >> 0u) & 0xFFu);
    float g = float((rgba >> 8u) & 0xFFu);
    float b = float((rgba >> 16u) & 0xFFu);
    float a = float((rgba >> 24u) & 0xFFu);
    return float4(r / 255.0, g / 255.0, b / 255.0, a / 255.0);
}

#endif // __cplusplus

#endif // shaderSharedUtils_h
