#ifndef jfa_h
#define jfa_h

#ifdef __cplusplus

#include <glm/ext/matrix_float3x4.hpp>
#include <glm/glm.hpp>

using mat4 = glm::mat4;
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;

#endif

struct JFAPushConstants {
    uint step;
    uint swapInputAndOutput;
};

static constexpr uint MAX_UINT = 60000;

#endif