import layoutAndBindings;
import common;
import uniforms;
import sharedUtils;
import pbr;
import math;
import light;

struct FragmentInput {
    float4 position : SV_Position;
    float4 tangent : TANGENT;
    float3 worldPosition : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float2 uv2 : TEXCOORD1;
    uint packedColor : PACKED_COLOR;
    // float3 baryCoords : SV_Barycentrics;
    nointerpolation uint instanceIndex : INSTANCE_INDEX;
    nointerpolation uint drawIndex : DRAW_INDEX;
};

float edgeFactor(float3 barycoords, float thickness) {
    float edgeIntensity = min(barycoords.x, min(barycoords.y, barycoords.z));
    float lineSmoothing = fwidth(edgeIntensity);
    return smoothstep(0.0, lineSmoothing * thickness, edgeIntensity);
}

// ACES filmic tone mapping (approximation by Krzysztof Narkowicz)
float3 ACESFilm(float3 x) {
    const float a = 2.51f;
    const float b = 0.03f;
    const float c = 2.43f;
    const float d = 0.59f;
    const float e = 0.14f;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0f, 1.0f);
}

struct FragmentOutput
{
    float4 color : SV_Target0;

    uint objectId : SV_Target1;
};

[shader("fragment")]
FragmentOutput fragMain(uniform GlobalPushConstants pushConstants, FragmentInput input) {

    uint instanceIndex = input.instanceIndex;
    uint drawIndex = input.drawIndex;
    PerDrawData drawData = drawData[drawIndex];
    PBRMaterial material = materials[drawData.materialIndex];
    float2[2] UVs = float2[2](input.uv, input.uv2);
    InstanceData instance = instances[instanceIndex];
    float3x3 normalMatrix = float3x3(instance.normalMatrix);

    float4 albedo = sampleBaseColor(material, UVs[material.baseColorTextureUVIndex]);
    float4 metallicRoughness = sampleMetallicRoughness(material, UVs[material.metallicRoughnessTextureUVIndex]);
    float4 ao = sampleAmbientOcclusion(material, UVs[material.aoTextureUVIndex]);
    float4 emissive = sampleEmissive(material, UVs[material.emissiveTextureUVIndex]);

    float3 finalNormal = input.normal;
    float2 normalUV = UVs[material.normalMapTextureUVIndex];
    float4 sampledNormal = sampleNormalMap(material, normalUV);
    if ((drawData.flags & ObjectFlags::NoTangent) != 0) {
        finalNormal = normalize(mul(normalMatrix, finalNormal));
        finalNormal = perturbNormal(finalNormal, input.worldPosition, sampledNormal.xyz, normalUV);
    } else {
        float3 normal = normalize(mul(normalMatrix, input.normal));
        float3 tangent = normalize(mul(normalMatrix, input.tangent.xyz));

        tangent = normalize(tangent - dot(tangent, normal) * normal);

        float3 edgeTangent = tangent - dot(tangent, normal) * normal;

        if (dot(edgeTangent, edgeTangent) > 0.001f) {
            tangent = normalize(edgeTangent);
        } else {
            tangent = float3(1.0f, 0.0f, 0.0f); 
            if (abs(dot(tangent, normal)) > 0.99f) {
                tangent = float3(0.0f, 0.0f, 1.0f);
            }
            tangent = normalize(tangent - dot(tangent, normal) * normal);
        }

        float3 bitangent = cross(normal, tangent) * input.tangent.w;
        float3x3 TBN = float3x3(tangent, bitangent, normal);

        finalNormal = normalize(mul(TBN, sampledNormal.xyz * 2.0 - 1.0));
    }

    PBRProperties pbrProperties = calculatePBRPropertiesMetallicRoughness(
        material,
        albedo,
        finalNormal,
        ubo.cameraPosition.xyz,
        input.worldPosition,
        metallicRoughness);

    float3 specularColor = getIBLRadianceContributionGGX(environmentMaps[ubo.environmentMapIndex], pbrProperties, 1.0);
    float3 diffuseColor = getIBLRadianceLambertian(environmentMaps[ubo.environmentMapIndex], pbrProperties, 1.0);
    float3 ambientColor = diffuseColor + specularColor;

    LightData sunLight = lights.lights[0];

    float3 lightDirection = normalize(-sunLight.position.xyz);

    float shadowFactor = calculateSunShadow(input.worldPosition, finalNormal, lightDirection, sunLight);

    float3 directLight = calculatePBRLightContribution(
        pbrProperties, normalize(sunLight.position.xyz - input.worldPosition), sunLight.color.rgb
    );

    float ambientShadow = lerp(0.6f, 0.9f, shadowFactor);
    float directShadow = lerp(0.2f, 0.3f, shadowFactor);
    float3 color = ambientColor * ambientShadow + directLight * directShadow;

    color = color * (ao.r < 0.01f ? 1.0f : ao.r);
    color = emissive.rgb + color;

    // color = ACESFilm(color);

    float4 finalColor;
    if ((drawData.flags & ObjectFlags::IsWireframe) != 0) {
        // float val = edgeFactor(input.baryCoords, 1.0);
        // return lerp(albedo, float4(0, 0, 0, 1), 1.0 - val);
        finalColor = float4(1.0f, 0.0f, 0.0f, 1.0f);
    } else {
        finalColor = float4(color, 1.0f);
    }
    uint isInstanceSelected = (instance.flags & InstanceFlags::IsInstanceSelected) != 0u;
    uint isNoId = instance.instanceId == NO_ID;
    uint hasMaskId = isInstanceSelected * (1u - isNoId);
    uint finalId = instanceIndex * hasMaskId + NO_ID * (1u - hasMaskId);
    float4 shadowColor = float4(0, shadowFactor, shadowFactor, 1.0f);
    return FragmentOutput(finalColor, finalId);
}
