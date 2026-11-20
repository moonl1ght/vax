#pragma once

#include <cstdint>
#include <limits>

namespace vax::rhi {
constexpr uint32_t NullId = std::numeric_limits<uint32_t>::max();

using MeshId = uint32_t;
constexpr MeshId NullMeshId = std::numeric_limits<MeshId>::max();
using BufferId = uint32_t;
constexpr BufferId NullBufferId = std::numeric_limits<BufferId>::max();
using MaterialId = uint32_t;
constexpr MaterialId NullMaterialId = std::numeric_limits<MaterialId>::max();
} // namespace vax::rhi