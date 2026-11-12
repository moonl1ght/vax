module layout;

#include "shaderUniforms.h"
#include "shaderConstants.h"

[vk::binding(SAMPLED_IMAGE_BINDING_INDEX, BINDLESS_SET_INDEX)]
__DynamicResource<__DynamicResourceKind.General> sampledImages[];

[vk::binding(STORAGE_IMAGE_BINDING_INDEX, BINDLESS_SET_INDEX)]
__DynamicResource<__DynamicResourceKind.General> storageImages[];

[vk::binding(SAMPLER_BINDING_INDEX, BINDLESS_SET_INDEX)]
__DynamicResource<__DynamicResourceKind.Sampler> samplers[];

[vk::binding(STORAGE_BUFFER_BINDING_INDEX, BINDLESS_SET_INDEX)]
__DynamicResource<__DynamicResourceKind.General> storageBuffers[];

[vk::binding(FRAME_UNIFORM_BUFFER_INDEX, UBO_SET_INDEX)]
ConstantBuffer<UniformBufferObject> frameUniformBuffer;

[vk::binding(LIGHT_UNIFORM_BUFFER_INDEX, UBO_SET_INDEX)]
ConstantBuffer<LightUBO> lightUniformBuffer;

export T getDescriptorFromHandle<T>(DescriptorHandle<T> handle)
    where T : IOpaqueDescriptor {
    __target_switch {
    case spirv:
        let index = ((uint2)handle).x;
        if (T.kind == DescriptorKind.Sampler)
            return samplers[index].asOpaqueDescriptor<T>();
        if (T.kind == DescriptorKind.Buffer)
            return storageBuffers[index].asOpaqueDescriptor<T>();
        if (T.descriptorAccess == DescriptorAccess.Read)
            return sampledImages[index].asOpaqueDescriptor<T>();
        return storageImages[index].asOpaqueDescriptor<T>();
    default:
        return defaultGetDescriptorFromHandle(handle);
    }
}
