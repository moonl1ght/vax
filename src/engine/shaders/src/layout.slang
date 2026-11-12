module layout;

[vk::binding(0, 0)]
__DynamicResource<__DynamicResourceKind.General> sampledImages[];

[vk::binding(1, 0)]
__DynamicResource<__DynamicResourceKind.General> storageImages[];

export T getDescriptorFromHandle<T>(DescriptorHandle<T> handle)
    where T : IOpaqueDescriptor {
    __target_switch {
    case spirv:
        if (T.descriptorAccess == DescriptorAccess.Read)
            return sampledImages[((uint2)handle).x].asOpaqueDescriptor<T>();
        else
            return storageImages[((uint2)handle).x].asOpaqueDescriptor<T>();
    default:
        return defaultGetDescriptorFromHandle(handle);
    }
}
