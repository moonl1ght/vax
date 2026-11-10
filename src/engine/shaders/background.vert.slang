import layoutAndBindings;

struct Coordinates {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

[shader("vertex")]
Coordinates vertMain(uint vertexIndex: SV_VulkanVertexID) {
    Coordinates output;
    float2 uv = float2((vertexIndex << 1) & 2, vertexIndex & 2);
    output.position = float4(uv * 2.0 - 1.0, 0.0, 1.0);
    output.uv = uv;
    return output;
}
