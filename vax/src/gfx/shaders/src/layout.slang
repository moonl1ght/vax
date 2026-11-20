module layout;

import sharedTypes;

[vk::binding(SAMPLED_IMAGE_BINDING_INDEX, BINDLESS_SET_INDEX)]
__DynamicResource<__DynamicResourceKind.General> sampledImages[];

[vk::binding(STORAGE_IMAGE_BINDING_INDEX, BINDLESS_SET_INDEX)]
__DynamicResource<__DynamicResourceKind.General> storageImages[];

[vk::binding(SAMPLER_BINDING_INDEX, BINDLESS_SET_INDEX)]
__DynamicResource<__DynamicResourceKind.Sampler> samplers[];

[vk::binding(STORAGE_BUFFER_BINDING_INDEX, BINDLESS_SET_INDEX)]
__DynamicResource<__DynamicResourceKind.General> storageBuffers[];

[vk::binding(FRAME_UNIFORM_BUFFER_INDEX, UBO_SET_INDEX)]
public ConstantBuffer<UniformBufferObject> frameUniformBuffer;

[vk::binding(LIGHT_UNIFORM_BUFFER_INDEX, UBO_SET_INDEX)]
public ConstantBuffer<LightUBO> lightUniformBuffer;

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

public struct VertexOutput {
    public float4 position : SV_Position;

    public float4 tangent : TANGENT;

    public float3 worldPosition : WORLD_POSITION;

    public float3 normal : NORMAL;

    public float2 uv : TEXCOORD0;

    public float2 uv2 : TEXCOORD1;

    public uint packedColor : PACKED_COLOR;

    public nointerpolation uint instanceIndex : INSTANCE_INDEX;

    public nointerpolation uint drawIndex : DRAW_INDEX;
};
