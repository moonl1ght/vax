module simple;

import sharedTypes;
import layout;
import base;
import utils.sampling;

struct FragmentInput {
    float4 position : SV_Position;
    float4 tangent : TANGENT;
    float3 worldPosition : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float2 uv2 : TEXCOORD1;
    uint packedColor : PACKED_COLOR;
    nointerpolation uint instanceIndex : INSTANCE_INDEX;
    nointerpolation uint drawIndex : DRAW_INDEX;
};

[shader("fragment")]
float4 fragment_main(uniform SimplePushConstants pushConstants, FragmentInput input)
    : SV_Target {
    PerDrawData drawData = pushConstants.drawData[input.drawIndex];
    PBRMaterial material = pushConstants.materials[drawData.materialIndex];

    float4 albedo = sampleBaseColor(material, input.uv);

    return albedo;
}

[shader("vertex")]
VertexOutput vertex_main(
    uniform SimplePushConstants pushConstants,
    uint vertexIndex: SV_VulkanVertexID,
    uint instanceIndex: SV_VulkanInstanceID,
    uint drawIndex: SV_DrawIndex
) {
    return BaseVertexOutput::calculate(
        pushConstants.vertices,
        pushConstants.instances,
        pushConstants.drawData,
        vertexIndex,
        instanceIndex,
        drawIndex,
        pushConstants.drawIndexOffset
    );
}
