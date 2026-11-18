#ifndef simple_h
#define simple_h

#include "shaderData.h"
#include "shaderResourceHandle.h"
#include "vertex.h"

struct SimplePushConstants {
    STRUCTURED_BUFFER_HANDLE(StructuredBuffer<Vertex>) vertices;
    STRUCTURED_BUFFER_HANDLE(StructuredBuffer<InstanceData>) instances;
    STRUCTURED_BUFFER_HANDLE(StructuredBuffer<PerDrawData>) drawData;
    STRUCTURED_BUFFER_HANDLE(StructuredBuffer<PBRMaterial>) materials;
    uint32_t drawIndexOffset;
};

#endif