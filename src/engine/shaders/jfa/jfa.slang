#include "jfa.h"

import uniforms;
import utils.customAttributes;

struct JFAInit {
    Texture2D<uint> inputMask;
    Texture2D<float> inputDepth;
    RWTexture2D<uint2> textureA;

    void execute(uint3 dtid) {
        uint2 pixelCoords = dtid.xy;

        uint objectId = inputMask.Load(int3(pixelCoords, 0));

        uint width, height;
        inputMask.GetDimensions(width, height);
        if (pixelCoords.x >= width || pixelCoords.y >= height)
            return;

        float currentDepth = inputDepth.Load(int3(pixelCoords, 0));

        // uint isSelected = objectId > 0;
        // uint2 point = isSelected * pixelCoords + (1 - isSelected) * uint2(60000, 60000);

        // textureA[pixelCoords] = point;
        bool isEdge = false;

        if (objectId != NO_ID) {
            int2 offsets[4] = { int2(1, 0), int2(-1, 0), int2(0, 1), int2(0, -1) };

            for (int i = 0; i < 4; ++i) {
                int2 neighborCoords = int2(pixelCoords) + offsets[i];

                if (neighborCoords.x >= 0 && neighborCoords.x < int(width) && neighborCoords.y >= 0 &&
                    neighborCoords.y < int(height)) {
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
};

struct JFAStep {
    RWTexture2D<uint2> inputCoords;
    RWTexture2D<uint2> outputCoords;

    void execute(uint3 dtid, JFAPushConstants pc) {
        uint2 pixelCoords = dtid.xy;

        uint width, height;
        RWTexture2D<uint2> currentInputCoords = pc.swapInputAndOutput == 0u ? inputCoords : outputCoords;
        RWTexture2D<uint2> currentOutputCoords = pc.swapInputAndOutput == 0u ? outputCoords : inputCoords;
        currentInputCoords.GetDimensions(width, height);

        if (pixelCoords.x >= width || pixelCoords.y >= height)
            return;

        uint2 bestSeed = uint2(MAX_UINT, MAX_UINT);
        float minDistance = float.maxValue;

        for (int y = -1; y <= 1; ++y) {
            for (int x = -1; x <= 1; ++x) {
                int2 neighborCoords = int2(pixelCoords) + int2(x, y) * int(pc.step);

                if (neighborCoords.x >= 0 && neighborCoords.x < int(width) && neighborCoords.y >= 0 &&
                    neighborCoords.y < int(height)) {
                    uint2 seed = currentInputCoords[neighborCoords];

                    if (seed.x != MAX_UINT && seed.y != MAX_UINT) {
                        float2 diff = float2(pixelCoords) - float2(seed);

                        float distanceSqr = diff.x * diff.x + diff.y * diff.y;

                        if (distanceSqr < minDistance) {
                            minDistance = distanceSqr;
                            bestSeed = seed;
                        }
                    }
                }
            }
        }

        currentOutputCoords[pixelCoords] = bestSeed;
    }
};

[shader("compute")]
[numthreads(16, 16, 1)]
void jfa_init(uint3 dtid: SV_DispatchThreadID, [NoUAB] uniform ParameterBlock<JFAInit> block) { block.execute(dtid); }

[shader("compute")]
[numthreads(16, 16, 1)]
void jfa_main(uint3 dtid: SV_DispatchThreadID, uniform JFAPushConstants pc, [
    NoUAB
] uniform ParameterBlock<JFAStep> block) {
    block.execute(dtid, pc);
}
