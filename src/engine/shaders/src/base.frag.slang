import layoutAndBindings;
import common;
import uniforms;
import sharedUtils;
import pbr;
import math;

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
float4 fragMain(uniform GlobalPushConstants pushConstants, FragmentInput input)
    : SV_Target {
    PerDrawData drawData = drawData[input.drawIndex];
    PBRMaterial material = materials[drawData.materialIndex];

    float4 albedo = sampleBaseColor(material, input.uv);

    return albedo;
}