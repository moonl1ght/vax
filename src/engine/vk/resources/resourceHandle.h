#pragma once

#include "resourceUtils.h"

namespace vax::vk {
template <typename T, T NullId> class ResourceHandle final {
  public:
    ResourceHandle(T id)
        : _id(id) {};

    ResourceHandle()
        : _id(NullId) {};

    T id() const { return _id; }

  private:
    T _id = NullId;
};

using MeshHandle = ResourceHandle<MeshId, NullMeshId>;
using BufferHandle = ResourceHandle<BufferId, NullBufferId>;
using TextureHandle = ResourceHandle<TextureId, NullTextureId>;
using SamplerHandle = ResourceHandle<SamplerId, NullSamplerId>;
} // namespace vax::vk
