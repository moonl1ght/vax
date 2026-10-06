#pragma once

#include "colorPalette.h"
#include "transform.h"

namespace vax::engine {
struct PrefabDescriptor {
    enum class ModelExtension { GLB, URDF, UNKNOWN };

    enum class ModelType { MODEL, PRIMITIVE_CUBE, PRIMITIVE_PLANE };

    struct PrimitiveDescriptor {
        float size = 1.0f;
        Color color = ColorPalette::White;
    };

    struct SelectedInstanceInfo {
        uint32_t instanceIndex;
        Color color = ColorPalette::Clear;
    };

    PrimitiveDescriptor primitiveDescriptor;
    std::string path;
    std::string id;
    std::vector<vax::math::Transform> transforms;
    std::vector<SelectedInstanceInfo> selectedInstanceInfos;
    ModelType modelType;
    uint32_t instancesCount;
    bool isIdentifiable;

    ModelExtension getModelExtension() const;

    const std::string_view getMainPath() const;
};
} // namespace vax::engine