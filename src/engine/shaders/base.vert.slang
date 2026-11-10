import layoutAndBindings;
import sharedUtils;
import uniforms;

[shader("vertex")]
VertexOutput vertMain(
    uniform GlobalPushConstants pushConstants,
    uint vertexIndex: SV_VulkanVertexID,
    uint instanceIndex: SV_VulkanInstanceID,
    uint drawIndex: SV_DrawIndex
) {
    VertexInput input = vertices[vertexIndex];
    VertexOutput output;
    InstanceData instance = instances[instanceIndex];
    float4x4 modelMatrix = instance.model;
    uint absoluteDrawIndex = pushConstants.drawIndexOffset + drawIndex;
    PerDrawData drawData = drawData[absoluteDrawIndex];
    if ((drawData.flags & ObjectFlags::PrecomputedMVP) != 0) {
        output.position = mul(modelMatrix, float4(input.position, 1.0));
    } else {
        output.position = mul(ubo.proj, mul(ubo.view, mul(modelMatrix, float4(input.position, 1.0))));
    }
    output.tangent = input.tangent;
    output.worldPosition = mul(modelMatrix, float4(input.position, 1.0)).xyz;
    output.normal = input.normal;
    output.uv = input.uv;
    output.uv2 = input.uv2;
    output.packedColor = input.packedColor;
    output.instanceIndex = instanceIndex;
    output.drawIndex = absoluteDrawIndex;
    return output;
}
