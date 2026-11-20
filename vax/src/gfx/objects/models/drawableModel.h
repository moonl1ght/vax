#pragma once

#include "indirectDrawController.h"
#include "logger.h"
#include "mesh.h"
#include "resourceHandle.h"
#include "submesh.h"
#include <cstdint>
#include <utility>
#include <vector>

namespace vax::gfx {
class PrimitivesBuilder;
class ModelLoader;
} // namespace vax::gfx

namespace vax::gfx {
class DrawableModel final {
  public:
    struct DrawSettings {
        uint32_t instanceOffset;
        uint32_t instancesCount;
    };

    struct Settings {
        bool useWireframe = false;
        bool hasTangents = false;
        bool skipPushConstants = false;
        bool precomputedMVP = false;
    };

    friend class vax::gfx::PrimitivesBuilder;
    friend class vax::gfx::ModelLoader;

    explicit DrawableModel(vax::vk::MeshManager& meshManager, vax::vk::MeshHandle meshHandle)
        : _meshManager(meshManager)
        , _meshHandle(meshHandle) {};

    DrawableModel(DrawableModel&& other) noexcept = default;
    DrawableModel& operator=(DrawableModel&& other) noexcept = default;

    DrawableModel(const DrawableModel& other) = delete;
    DrawableModel& operator=(const DrawableModel& other) = delete;

    ~DrawableModel() {};

    void prepareDrawing(
        gfx::IndirectDrawController* indirectDrawController, uint32_t frameIndex, const DrawSettings& drawSettings
    );

    Settings& settings() { return _settings; }

    const Settings& settings() const { return _settings; }

    void setSettings(const Settings& settings) { _settings = settings; }

    vax::vk::Submesh& submesh(size_t index) { return _submeshes[index]; }

    const vax::vk::Submesh& submesh(size_t index) const { return _submeshes[index]; }

    size_t submeshCount() const { return _submeshes.size(); }

  private:
    vax::Logger _logger = vax::Logger("DrawableModel");

    std::reference_wrapper<vax::vk::MeshManager> _meshManager;

    vax::vk::MeshHandle _meshHandle;

    // TODO: remove this will need to use mesh manager to get the mesh
    vax::vk::Mesh* _mesh;
    std::vector<vax::vk::Submesh> _submeshes;
    Settings _settings;
};

struct DrawableModelHandle final {
    using InstanceDrawingRange = std::pair<uint32_t, uint32_t>;

    DrawableModel* drawableModel;

    std::vector<InstanceDrawingRange> instanceDrawingRanges;
};
} // namespace vax::gfx