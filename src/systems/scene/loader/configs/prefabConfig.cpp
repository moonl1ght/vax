#include "prefabConfig.h"

using namespace vax::scene;

PrefabConfig::AssetExtension PrefabConfig::AssetDescriptor::getAssetExtension() const {
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

const std::string_view PrefabConfig::AssetDescriptor::getMainPath() const {
    auto dot = path.rfind('/');
    if (dot == std::string::npos)
        return std::string_view("");
    return std::string_view(path).substr(0, dot);
}