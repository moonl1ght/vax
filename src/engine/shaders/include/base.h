#ifndef base_h
#define base_h

#include "shaderData.h"
#include "shaderResourceHandle.h"
#include "vertex.h"

struct BasePushConstants {
    STRUCTURED_BUFFER_HANDLE(StructuredBuffer<Vertex>) vertices;
    STRUCTURED_BUFFER_HANDLE(StructuredBuffer<InstanceData>) instances;
    STRUCTURED_BUFFER_HANDLE(StructuredBuffer<PerDrawData>) drawData;
    uint32_t drawIndexOffset;
};

#endif