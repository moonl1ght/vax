#include "jfa/jfa.h"
import layoutAndBindings;
import uniforms;
import sharedUtils;

[vk::binding(0, 0)]
Sampler2D inputTexture;

[vk::binding(1, 0)]
Texture2D<uint> inputMask;

[vk::binding(2, 0)]
Texture2D<float> inputDepth;

[vk::binding(0, 1)]
Texture2D<uint2> inputJFATexture;

[vk::binding(PerFrameDescriptorSetResourceIndex::FRAME_INSTANCE_BUFFER_INDEX, 2)]
StructuredBuffer<InstanceData> instancesFinalBlend;

[shader("fragment")]
float4 fragMain(float2 uv: TEXCOORD0) : SV_Target {
    float3 topColor = float3(0.1f, 0.2f, 0.4f);
    float3 bottomColor = float3(0.55f, 0.55f, 0.55f);

    float3 gradient = lerp(bottomColor, topColor, uv.y);

    uv.y = 1.0f - uv.y;
    float4 sceneColor = inputTexture.Sample(uv);

    uint width, height;
    inputJFATexture.GetDimensions(width, height);

    uint2 pixelCoords = uint2(uv * float2(width, height));
    uint2 closestEdgePixel = inputJFATexture[pixelCoords];

    float4 finalColor = sceneColor;
    if (closestEdgePixel.x != MAX_UINT) {
        float dist = distance(float2(pixelCoords), float2(closestEdgePixel));

        float outlineWidth = 5.0f;
        uint currentObjId = inputMask[pixelCoords];
        if (dist > 0.0f && dist <= outlineWidth) {

            float currentDepth = inputDepth.Load(int3(pixelCoords, 0));
            float edgeDepth = inputDepth.Load(int3(closestEdgePixel, 0));

            if (currentDepth >= edgeDepth + 0.001f) {
                return lerp(float4(gradient, 1.0f), sceneColor, sceneColor.a);
            }

            uint edgeObjId = inputMask[closestEdgePixel];
            float4 outlineColor = unpackRGBA(instancesFinalBlend[edgeObjId].packedColor);

            bool drawOutline = false;

            if (currentObjId != edgeObjId) {
               drawOutline = true;
            }

            if (drawOutline) {
                float edgeAlpha = smoothstep(outlineWidth, 0, dist);
                float finalAlpha = outlineColor.a * edgeAlpha;
                finalColor = float4(lerp(sceneColor.rgb, outlineColor.rgb, finalAlpha), sceneColor.a);
            }
        }
    }

    return lerp(float4(gradient, 1.0f), finalColor, sceneColor.a);
}