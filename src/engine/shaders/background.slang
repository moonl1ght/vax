struct Coordinates {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

[shader("vertex")]
Coordinates vertex_main(uint vertexIndex: SV_VulkanVertexID) {
    Coordinates output;
    float2 uv = float2((vertexIndex << 1) & 2, vertexIndex & 2);
    output.position = float4(uv * 2.0 - 1.0, 0.0, 1.0);
    output.uv = uv;
    return output;
}

[shader("fragment")]
float4 fragment_main(float2 uv: TEXCOORD0)
    : SV_Target {
    float3 topColor = float3(0.1f, 0.2f, 0.4f);
    float3 bottomColor = float3(0.55f, 0.55f, 0.55f);

    float3 gradient = lerp(bottomColor, topColor, uv.y);

    return float4(gradient, 1.0f);
}
