import layoutAndBindings;
import uniforms;
import sharedUtils;

[vk::binding(0, 0)]
Sampler2D inputTexture;

[shader("fragment")]
float4 fragMain(float2 uv: TEXCOORD0) : SV_Target {
    float3 topColor = float3(0.1f, 0.2f, 0.4f);
    float3 bottomColor = float3(0.55f, 0.55f, 0.55f);

    float3 gradient = lerp(bottomColor, topColor, uv.y);

    uv.y = 1.0f - uv.y;
    float4 sceneColor = inputTexture.Sample(uv);

    return lerp(float4(gradient, 1.0f), sceneColor, sceneColor.a);
}