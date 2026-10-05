#include "modelDescriptor.h"

using namespace vax::engine;

ModelDescriptor::ModelExtension ModelDescriptor::getModelExtension() const {
    auto dot = path.rfind('.');
    if (dot == std::string::npos)
        return ModelExtension::UNKNOWN;

    std::string ext = path.substr(dot + 1);
    for (auto& c : ext)
        c = std::tolower(c);

    if (ext == "glb")
        return ModelExtension::GLB;
    if (ext == "urdf")
        return ModelExtension::URDF;
    return ModelExtension::UNKNOWN;
}

const std::string_view ModelDescriptor::getMainPath() const {
    auto dot = path.rfind('/');
    if (dot == std::string::npos)
        return std::string_view("");
    return std::string_view(path).substr(0, dot);
}