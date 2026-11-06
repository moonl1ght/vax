#include "jfa.h"

import uniforms;

[vk::binding(0, 0)]
Texture2D<uint> inputMask;

[vk::binding(1, 0)]
Texture2D<float> inputDepth;

[vk::binding(2, 0)]
RWTexture2D<uint2> textureA;


[shader("compute")]
[numthreads(16, 16, 1)]
void compMain(uint3 dtid : SV_DispatchThreadID)
{
    uint2 pixelCoords = dtid.xy;

    uint objectId = inputMask.Load(int3(pixelCoords, 0));

    uint width, height;
    inputMask.GetDimensions(width, height);
    if (pixelCoords.x >= width || pixelCoords.y >= height) return;

    float currentDepth = inputDepth.Load(int3(pixelCoords, 0));

    // uint isSelected = objectId > 0;
    // uint2 point = isSelected * pixelCoords + (1 - isSelected) * uint2(60000, 60000);

    // textureA[pixelCoords] = point;
    bool isEdge = false;

    if (objectId != NO_ID) {
        int2 offsets[4] = { int2(1, 0), int2(-1, 0), int2(0, 1), int2(0, -1) };

        for (int i = 0; i < 4; ++i) {
            int2 neighborCoords = int2(pixelCoords) + offsets[i];

            if (neighborCoords.x >= 0 && neighborCoords.x < int(width) &&
                neighborCoords.y >= 0 && neighborCoords.y < int(height))
            {
                uint neighborId = inputMask.Load(int3(neighborCoords, 0));
                float neighborDepth = inputDepth.Load(int3(neighborCoords, 0));

                if (neighborId != objectId) {
                    if (neighborId == NO_ID || currentDepth == 0) {
                        isEdge = true;
                        break;
                    } else {
                        isEdge = currentDepth >= neighborDepth;
                    }
                }

                // isEdge = neighborId != objectId;

                // isEdge = isEdge && (neighborId == 0 || currentDepth == 0);

                if (isEdge) {
                    break;
                }
            } else {
                isEdge = false;
                break;
            }
        }
    }

    textureA[pixelCoords] = isEdge * pixelCoords + (1 - isEdge) * uint2(MAX_UINT, MAX_UINT);
}