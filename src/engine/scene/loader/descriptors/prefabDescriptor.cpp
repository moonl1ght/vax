#include "prefabDescriptor.h"

using namespace vax::engine;

PrefabDescriptor::AssetExtension PrefabDescriptor::AssetDescriptor::getAssetExtension() const {
    auto dot = path.rfind('.');
    if (dot == std::string::npos)
        return AssetExtension::UNKNOWN;

    std::string ext = path.substr(dot + 1);
    for (auto& c : ext)
        c = std::tolower(c);

    if (ext == "glb")
        return AssetExtension::GLB;
    if (ext == "urdf")
        return AssetExtension::URDF;
    return AssetExtension::UNKNOWN;
}

const std::string_view PrefabDescriptor::AssetDescriptor::getMainPath() const {
    auto dot = path.rfind('/');
    if (dot == std::string::npos)
        return std::string_view("");
    return std::string_view(path).substr(0, dot);
}