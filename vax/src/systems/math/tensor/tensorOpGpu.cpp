#include "tensorOp.h"

#include "add.cuh"
#include "logger.h"
#include "cudaUtils.h"

#include <cuda_runtime.h>

using namespace vax::math;

namespace {

const vax::Logger logger("TensorOpGpu");

bool check_broadcast_compatibility(const std::vector<int>& shapeA, const std::vector<int>& shapeB) {
    auto itA = shapeA.rbegin();
    auto itB = shapeB.rbegin();

    while (itA != shapeA.rend() && itB != shapeB.rend()) {
        int dimA = *itA;
        int dimB = *itB;

        // Rule: Dimensions must match, or one must be 1
        if (dimA != dimB && dimA != 1 && dimB != 1) {
            logger.error("Incompatible dimensions: ", dimA, " and ", dimB);
            return false;
        }

        itA++;
        itB++;
    }
    return true;
}

constexpr int kThreadsPerBlock = 256;
} // namespace

bool TensorOpGpu::add(Tensor& A, const Tensor& B) {
    if (A.totalSize() == 0 || B.totalSize() == 0) {
        logger.warning("add: A or B has size 0");
        return false;
    }
    auto broadcastCompatibility = check_broadcast_compatibility(A.shape(), B.shape());
    if (!broadcastCompatibility) {
        return false;
    }
    if (!A.isGpuAllocated() || !B.isGpuAllocated()) {
        logger.error("add: A or B has no device memory, call allocateGpuMemory/synchronizeHostToGpu first");
        return false;
    }

    launchAddInplace(A.gpuData(), B.gpuData(), A.totalSize(), B.totalSize(), kThreadsPerBlock);
    return cuda::check_launch("add");
}

bool TensorOpGpu::add(const Tensor& A, const Tensor& B, Tensor& C) {
    logger.warning("add: not implemented");
    return false;
}
