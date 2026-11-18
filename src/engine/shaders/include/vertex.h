#ifndef vertex_h
#define vertex_h

#ifdef __cplusplus

#include <glm/glm.hpp>

namespace vax::vk {
struct alignas(16) Vertex {
    static constexpr Vertex empty() {
        return Vertex{
            glm::vec3(0.0f, 0.0f, 0.0f),
            0,
            glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            0,
            glm::vec2(0.0f, 0.0f),
            glm::vec2(0.0f, 0.0f),
        };
    };

    glm::vec3 position;
    uint32_t packedColor;
    glm::vec4 tangent;
    glm::vec3 normal;
    uint32_t padding;
    glm::vec2 uv;
    glm::vec2 uv2;
};
} // namespace vax::vk

#else // Slang

struct Vertex {
    float3 position;

    uint packedColor;

    float4 tangent;

    float3 normal;

    uint padding;

    float2 uv;

    float2 uv2;
};

#endif

#endif // vertex_h