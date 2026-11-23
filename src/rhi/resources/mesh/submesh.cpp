#include "submesh.h"

using namespace vax::rhi;
using namespace vax;

void Submesh::debugPrint(const dev::Logger& logger) const {
    logger.debug(
        "Submesh: indexCount=",
        indexCount,
        ", firstIndex=",
        firstIndex,
        ", vertexOffset=",
        vertexOffset,
        ", materialIndex=",
        materialIndex
    );
}