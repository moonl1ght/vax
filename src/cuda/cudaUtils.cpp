#include "cudaUtils.h"

bool vax::cuda::check_launch(const char* op, const dev::Logger& logger) {
    auto launchResult = cudaGetLastError();
    if (launchResult != cudaSuccess) {
        logger.error(op, " launch failed: ", cudaGetErrorString(launchResult));
        return false;
    }
    return true;
}

bool vax::cuda::sync_device(const dev::Logger& logger) {
    auto syncResult = cudaDeviceSynchronize();
    if (syncResult != cudaSuccess) {
        logger.error("cudaDeviceSynchronize failed: ", cudaGetErrorString(syncResult));
        return false;
    }
    return true;
}