#include "finalblend.h"
#include "shaderSharedUtils.h"
#include "shaderConstants.h"

import layout;

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
float4 fragment_main(float2 uv: TEXCOORD0, uniform FinalBlendPushConstants pushConstants)
    : SV_Target {
    float3 topColor = float3(0.1f, 0.2f, 0.4f);
    float3 bottomColor = float3(0.55f, 0.55f, 0.55f);

    float3 gradient = lerp(bottomColor, topColor, uv.y);

    uv.y = 1.0f - uv.y;
    float4 sceneColor = pushConstants.inputTexture.Sample(pushConstants.inputTextureSampler, uv);

    uint width, height;
    pushConstants.inputJFATexture.GetDimensions(width, height);

    uint2 pixelCoords = uint2(uv * float2(width, height));
    uint2 closestEdgePixel = pushConstants.inputJFATexture[pixelCoords];

    float4 finalColor = sceneColor;
    if (closestEdgePixel.x != MAX_UINT) {
        float dist = distance(float2(pixelCoords), float2(closestEdgePixel));

        float outlineWidth = 5.0f;
        uint currentObjId = pushConstants.inputMask[pixelCoords];
        if (dist > 0.0f && dist <= outlineWidth) {

            float currentDepth = pushConstants.inputDepth.Load(int3(pixelCoords, 0));
            float edgeDepth = pushConstants.inputDepth.Load(int3(closestEdgePixel, 0));

            if (currentDepth >= edgeDepth + 0.001f) {
                return lerp(float4(gradient, 1.0f), sceneColor, sceneColor.a);
            }

            uint edgeObjId = pushConstants.inputMask[closestEdgePixel];
            float4 outlineColor = unpackRGBA(pushConstants.instances[edgeObjId].packedColor);

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
