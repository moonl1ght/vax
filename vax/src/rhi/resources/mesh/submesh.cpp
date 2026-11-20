#include "submesh.h"

using namespace vax::vk;
using namespace vax;

void Submesh::debugPrint(const Logger& logger) const {
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