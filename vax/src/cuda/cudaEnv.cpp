#include "cudaEnv.h"

using namespace vax;

std::string version(int packed) { return std::to_string(packed / 1000) + "." + std::to_string((packed % 1000) / 10); }

void CudaEnv::printInfo() const {
    vax::Logger logger = vax::Logger("CudaEnv");
    logger.info("===== CUDA =====");
    logger.info("Driver version:               ", version(_driverVersion));
    logger.info("Runtime version:              ", version(_runtimeVersion));
    logger.info("Device count:                 ", _devices.size());

    for (const auto& device : _devices) {
        device.printGpuInfo();
    }

    logger.info("================");
}

void CudaEnv::_load() {
    vax::Logger logger = vax::Logger("CudaEnv");
    int deviceCount = 0;
    auto countResult = cudaGetDeviceCount(&deviceCount);
    if (countResult != cudaSuccess) {
        return;
    }
    if (deviceCount == 0) {
        logger.warning("No CUDA capable devices found");
        return;
    }

    cudaDriverGetVersion(&_driverVersion);
    cudaRuntimeGetVersion(&_runtimeVersion);

    for (int i = 0; i < deviceCount; ++i) {
        auto device = CudaDevice(i);
        if (device.deviceExists()) {
            _devices.push_back(device);
        }
    }
}