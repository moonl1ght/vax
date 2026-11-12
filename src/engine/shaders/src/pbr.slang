import uniforms;
import math;
import common;

struct PBRProperties
{
    // geometry properties
    float NdotL; // cos angle between normal and light direction
    float NdotV; // cos angle between normal and view direction
    float NdotH; // cos angle between normal and half vector
    float LdotH; // cos angle between light direction and half vector
    float VdotH; // cos angle between view direction and half vector
    float3 n;    // normal at surface point
    float3 v;    // vector from surface point to camera

    // material properties
    float roughnessFactor; // roughness value, input to shader
    float3 reflectance0;   // full reflectance color (normal incidence angle)
    float3 reflectance90;  // reflectance color at grazing angle
    float alphaRoughness;  // roughness mapped to a more linear change in the roughness
    float3 diffuseColor;   // color contribution from diffuse lighting
    float3 specularColor;  // color contribution from specular lighting
};

PBRProperties calculatePBRPropertiesMetallicRoughness(
    PBRMaterial material,
    float4 albedo,
    float3 normal,
    float3 cameraPos,
    float3 worldPos,
    float4 metallicRoughnessSample)
{
    PBRProperties pbrProperties;

    float roughnessFactor = material.roughnessFactor;
    float metallic = material.metallicFactor * metallicRoughnessSample.b;
    metallic = clamp(metallic, 0.0f, 1.0f);

    roughnessFactor = metallicRoughnessSample.g * roughnessFactor;
    roughnessFactor = clamp(roughnessFactor, 0.04f, 1.0f);

    float alphaRoughness = roughnessFactor * roughnessFactor;

    float4 baseColor = albedo;

    float3 f0 = float3(0.04f);
    float3 diffuseColor = lerp(baseColor.rgb, float3(0.0f, 0.0f, 0.0f), metallic);
    float3 specularColor = lerp(f0, baseColor.rgb, metallic);

    float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);

    float reflectance90 = clamp(reflectance * 25.0f, 0.0f, 1.0f);
    float3 specularEnvironmentR0 = specularColor.rgb;
    float3 specularEnvironmentR90 = float3(1.0f, 1.0f, 1.0f) * reflectance90;

    float3 n = normalize(normal);
    float3 v = normalize(cameraPos - worldPos);

    pbrProperties.NdotV = clamp(abs(dot(n, v)), 0.001f, 1.0f);
    pbrProperties.roughnessFactor = roughnessFactor;
    pbrProperties.reflectance0 = specularEnvironmentR0;
    pbrProperties.reflectance90 = specularEnvironmentR90;
    pbrProperties.alphaRoughness = alphaRoughness;
    pbrProperties.diffuseColor = diffuseColor;
    pbrProperties.specularColor = specularColor;
    pbrProperties.n = n;
    pbrProperties.v = v;

    return pbrProperties;
}

float3 getIBLRadianceLambertian(
    EnvironmentMapData environmentMapData, PBRProperties pbrProperties, float specularWeight
) {
    if (environmentMapData.envMapTextureIrradiance == NO_TEXTURE_FLAG)
        return float3(0.0f, 0.0f, 0.0f);
    if (environmentMapData.texBRDFLUT == NO_TEXTURE_FLAG)
        return float3(0.0f, 0.0f, 0.0f);

    float safeNdotV = clamp(pbrProperties.NdotV, 0.001f, 0.999f);
    float safeRoughness = clamp(pbrProperties.roughnessFactor, 0.0f, 1.0f);
    float2 brdfSamplePoint = float2(safeNdotV, safeRoughness);
    
    float2 f_ab = sampleBRDFLUT(brdfSamplePoint, environmentMapData).rg;

    float3 irradiance = sampleEnvMapIrradiance(pbrProperties.n, environmentMapData).rgb;

    float3 Fr = max(float3(1.0 - pbrProperties.roughnessFactor), pbrProperties.reflectance0) - pbrProperties.reflectance0;
    float3 k_S = pbrProperties.reflectance0 + Fr * pow(1.0 - pbrProperties.NdotV, 5.0);
    float3 FssEss = specularWeight * k_S * f_ab.x + f_ab.y;

    float Ems = (1.0 - (f_ab.x + f_ab.y));
    float3 F_avg = specularWeight * (pbrProperties.reflectance0 + (1.0 - pbrProperties.reflectance0) / 21.0);
    float3 FmsEms = Ems * FssEss * F_avg / (1.0 - F_avg * Ems);
    float3 k_D = pbrProperties.diffuseColor * (1.0 - FssEss + FmsEms);

    return (FmsEms + k_D) * irradiance;
}

float3 getIBLRadianceContributionGGX(
    EnvironmentMapData environmentMapData, PBRProperties pbrProperties, float specularWeight
) {
    if (environmentMapData.envMapTexture == NO_TEXTURE_FLAG)
        return float3(0.0f, 0.0f, 0.0f);
    if (environmentMapData.texBRDFLUT == NO_TEXTURE_FLAG)
        return float3(0.0f, 0.0f, 0.0f);
    float3 n = pbrProperties.n;
    float3 v = pbrProperties.v;
    float3 reflection = -normalize(reflect(v, n));

    int mipCount = sampleEnvMapQueryLevels(environmentMapData);
    float lod = pbrProperties.roughnessFactor * (mipCount - 1);
    float safeNdotV = clamp(pbrProperties.NdotV, 0.001f, 0.999f);
    float safeRoughness = clamp(pbrProperties.roughnessFactor, 0.0f, 1.0f);
    float2 brdfSamplePoint = float2(safeNdotV, safeRoughness);

    float3 brdf = sampleBRDFLUT(brdfSamplePoint, environmentMapData).rgb;
    float3 specularLight = sampleEnvMapLod(reflection, lod, environmentMapData).rgb;

    float3 Fr = max(float3(1.0f - pbrProperties.roughnessFactor), pbrProperties.reflectance0) - pbrProperties.reflectance0;
    float3 k_S = pbrProperties.reflectance0 + Fr * pow(1.0f - pbrProperties.NdotV, 5.0f);
    float3 FssEss = k_S * brdf.x + brdf.y;

    return specularWeight * specularLight * FssEss;
}

float3 specularReflection(PBRProperties pbrProperties)
{
    return pbrProperties.reflectance0 +
           (pbrProperties.reflectance90 - pbrProperties.reflectance0) *
               pow(clamp(1.0f - pbrProperties.VdotH, 0.0f, 1.0f), 5.0f);
}

float geometricOcclusion(PBRProperties pbrProperties)
{
    float NdotL = pbrProperties.NdotL;
    float NdotV = pbrProperties.NdotV;
    float rSqr = pbrProperties.alphaRoughness * pbrProperties.alphaRoughness;

    float attenuationL = 2.0f * NdotL / (NdotL + sqrt(rSqr + (1.0f - rSqr) * (NdotL * NdotL)));
    float attenuationV = 2.0f * NdotV / (NdotV + sqrt(rSqr + (1.0f - rSqr) * (NdotV * NdotV)));
    return attenuationL * attenuationV;
}

float microfacetDistribution(PBRProperties pbrProperties)
{
    float roughnessSq = pbrProperties.alphaRoughness * pbrProperties.alphaRoughness;
    float f = (pbrProperties.NdotH * roughnessSq - pbrProperties.NdotH) * pbrProperties.NdotH + 1.0f;
    return roughnessSq / (PI_F * f * f);
}

float3 diffuseBurley(PBRProperties pbrProperties)
{
    float f90 = 2.0f * pbrProperties.LdotH * pbrProperties.LdotH * pbrProperties.roughnessFactor - 0.5f;

    return (pbrProperties.diffuseColor / PI_F) *
           (1.0f + f90 * pow((1.0f - pbrProperties.NdotL), 5.0f)) *
           (1.0f + f90 * pow((1.0f - pbrProperties.NdotV), 5.0f));
}

float3 calculatePBRLightContribution(inout PBRProperties pbrProperties, float3 lightDirection, float3 lightColor)
{
    float3 n = pbrProperties.n;
    float3 v = pbrProperties.v;
    float3 ld = normalize(lightDirection);
    float3 h = normalize(ld + v);

    float NdotV = pbrProperties.NdotV;
    float NdotL = clamp(dot(n, ld), 0.001f, 1.0f);
    float NdotH = clamp(dot(n, h), 0.0f, 1.0f);
    float LdotH = clamp(dot(ld, h), 0.0f, 1.0f);
    float VdotH = clamp(dot(v, h), 0.0f, 1.0f);

    float3 color = float3(0.0f, 0.0f, 0.0f);

    if (NdotL > 0.0f || NdotV > 0.0f)
    {
        pbrProperties.NdotL = NdotL;
        pbrProperties.NdotH = NdotH;
        pbrProperties.LdotH = LdotH;
        pbrProperties.VdotH = VdotH;
        float3 F = specularReflection(pbrProperties);
        float G = geometricOcclusion(pbrProperties);
        float D = microfacetDistribution(pbrProperties);

        float3 diffuseContrib = (1.0f - F) * diffuseBurley(pbrProperties);
        float3 specContrib = F * G * D / (4.0f * NdotL * NdotV);
        color = NdotL * lightColor * (diffuseContrib + specContrib);
    }
    return color;
}
