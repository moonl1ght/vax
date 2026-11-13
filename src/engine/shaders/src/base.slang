module base;

import sharedTypes;
import layout;

public struct BaseVertexOutput {
    public static VertexOutput calculate(
        StructuredBuffer<Vertex>.Handle vertices,
        StructuredBuffer<InstanceData>.Handle instances,
        StructuredBuffer<PerDrawData>.Handle drawData,
        uint vertexIndex,
        uint instanceIndex,
        uint drawIndex,
        uint drawIndexOffset
    ) {
        Vertex input = vertices[vertexIndex];
        VertexOutput output;
        InstanceData instance = instances[instanceIndex];
        float4x4 modelMatrix = instance.model;
        uint absoluteDrawIndex = drawIndexOffset + drawIndex;
        PerDrawData drawData = drawData[absoluteDrawIndex];
        if ((drawData.flags & ObjectFlags::PrecomputedMVP) != 0) {
            output.position = mul(modelMatrix, float4(input.position, 1.0));
        } else {
            output.position = mul(frameUniformBuffer.proj, mul(frameUniformBuffer.view, mul(modelMatrix, float4(input.position, 1.0))));
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
}
