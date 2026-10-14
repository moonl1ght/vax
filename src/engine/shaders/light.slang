import uniforms;
import layoutAndBindings;

float calculateSunShadow(float3 worldPos, float3 normal, float3 lightDir, LightData light) {
    if (light.shadowMapIndex == NO_SHADOW_MAP_INDEX) {
        return 1.0f;
    }

    uint shadowMapWidth;
    uint shadowMapHeight;
    globalShadowTextures[light.shadowMapIndex].GetDimensions(shadowMapWidth, shadowMapHeight);
    float worldPerClip = 1.0f / length(light.lightSpaceMatrix[0].xyz);
    float texelWorldSize = 2.0f * worldPerClip / float(shadowMapWidth);
    float NdotL = saturate(dot(normal, -lightDir));
    float normalOffsetScale = 1.5f;
    float3 offsetPos = worldPos + normal * texelWorldSize * normalOffsetScale * (1.0f - NdotL);

    float4 lightSpacePos = mul(light.lightSpaceMatrix, float4(offsetPos, 1.0f));
    float3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    float2 shadowUV = projCoords.xy * 0.5f + 0.5f;

    shadowUV.y = 1.0f - shadowUV.y;

    if (projCoords.z > 1.0f || shadowUV.x < 0.0f || shadowUV.x > 1.0f || shadowUV.y < 0.0f || shadowUV.y > 1.0f) {
        return 1.0f;
    }

    float closestDepth = globalShadowTextures[light.shadowMapIndex].Sample(
        globalSamplers[light.shadowMapSamplerIndex],
        shadowUV
    ).r;

    float bias = max(0.000005f * (1.0f - NdotL), 0.00005f);

    float currentDepth = projCoords.z;
    float softness = 0.002f;
    float diff = closestDepth - (currentDepth + bias);
    float shadowFactor = 1.0f - clamp(diff / softness, 0.0f, 1.0f);

    float band = 0.1f;
    float fade = smoothstep(0.0f, band, NdotL);
    float sampleWeight = smoothstep(band, 2.0f * band, NdotL);

    return fade * lerp(1.0f, shadowFactor, sampleWeight);
}