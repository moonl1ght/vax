#include "cudaDevice.h"
#include "logger.h"

using namespace vax;

std::string mib(size_t bytes) { return std::to_string(bytes / (1024 * 1024)) + " MiB"; }

std::string kib(size_t bytes) { return std::to_string(bytes / 1024) + " KiB"; }

std::string dims(const int (&v)[3]) {
    return std::to_string(v[0]) + " x " + std::to_string(v[1]) + " x " + std::to_string(v[2]);
}

std::string yesNo(int value) { return value ? "yes" : "no"; }

int deviceAttribute(cudaDeviceAttr attr, int deviceIndex) {
    int value = 0;
    if (cudaDeviceGetAttribute(&value, attr, deviceIndex) != cudaSuccess) {
        return -1;
    }
    return value;
}

int cudaCoresPerSM(int major, int minor) {
    switch ((major << 4) + minor) {
    case 0x50:
    case 0x52:
    case 0x53:
        return 128; // Maxwell
    case 0x60:
        return 64; // Pascal GP100
    case 0x61:
    case 0x62:
        return 128; // Pascal
    case 0x70:
    case 0x72:
    case 0x75:
        return 64; // Volta / Turing
    case 0x80:
        return 64; // Ampere GA100
    case 0x86:
    case 0x87:
    case 0x89:
        return 128; // Ampere GA10x / Ada
    case 0x90:
        return 128; // Hopper
    case 0xa0:
    case 0xa1:
    case 0xc0:
    case 0xc1:
        return 128; // Blackwell
    default:
        return -1;
    }
}

void CudaDevice::printGpuInfo() const {
    auto logger = vax::Logger("CudaDevice");
    logger.info("----- Device ", _deviceId, ": ", _prop.name, " -----");
    logger.info("Compute capability:           ", _prop.major, ".", _prop.minor);
    logger.info("PCI (domain:bus:device):      ", _prop.pciDomainID, ":", _prop.pciBusID, ":", _prop.pciDeviceID);
    logger.info("Integrated:                   ", yesNo(_prop.integrated));

    if (_coresPerSM > 0) {
        logger.info("CUDA cores per SM:            ", _coresPerSM);
        logger.info("CUDA cores total:             ", _coresPerSM * _prop.multiProcessorCount);
    } else {
        logger.info("CUDA cores per SM:            unknown for this architecture");
    }

    logger.info("-- Memory --");
    logger.info("Total global memory:          ", mib(_prop.totalGlobalMem));
    logger.info("Free / total global memory:   ", mib(getMemoryUsage().first), " / ", mib(getMemoryUsage().second));
    logger.info("Memory bus width:             ", _prop.memoryBusWidth, " bit");
    if (_memoryClockKHz > 0) {
        logger.info("Memory clock rate:            ", _memoryClockKHz / 1000, " MHz");
        logger.info("Peak memory bandwidth:        ", static_cast<int>(_bandwidth), " GB/s");
    }
    logger.info("L2 cache size:                ", kib(static_cast<size_t>(_prop.l2CacheSize)));
    logger.info("Persisting L2 max size:       ", kib(static_cast<size_t>(_prop.persistingL2CacheMaxSize)));
    logger.info("Shared memory per block:      ", kib(_prop.sharedMemPerBlock));
    logger.info("Shared mem per block (optin): ", kib(_prop.sharedMemPerBlockOptin));
    logger.info("Shared memory per SM:         ", kib(_prop.sharedMemPerMultiprocessor));
    logger.info("Reserved shared mem/block:    ", _prop.reservedSharedMemPerBlock, " B");
    logger.info("Constant memory:              ", kib(_prop.totalConstMem));
    logger.info("Registers per block:          ", _prop.regsPerBlock);
    logger.info("Registers per SM:             ", _prop.regsPerMultiprocessor);
    logger.info("Max memcpy pitch:             ", mib(_prop.memPitch));
    logger.info("Texture alignment:            ", _prop.textureAlignment, " B");
    logger.info("ECC enabled:                  ", yesNo(_prop.ECCEnabled));

    logger.info("-- Capabilities --");
    logger.info("Concurrent kernels:           ", yesNo(_prop.concurrentKernels));
    logger.info("Async engines (copy):         ", _prop.asyncEngineCount);
    logger.info("Cooperative launch:           ", yesNo(_prop.cooperativeLaunch));
    logger.info("Cluster launch:               ", yesNo(_prop.clusterLaunch));
    logger.info("Stream priorities:            ", yesNo(_prop.streamPrioritiesSupported));
    logger.info("Unified addressing:           ", yesNo(_prop.unifiedAddressing));
    logger.info("Managed memory:               ", yesNo(_prop.managedMemory));
    logger.info("Concurrent managed access:    ", yesNo(_prop.concurrentManagedAccess));
    logger.info("Pageable memory access:       ", yesNo(_prop.pageableMemoryAccess));
    logger.info("Can map host memory:          ", yesNo(_prop.canMapHostMemory));
    logger.info("Host register supported:      ", yesNo(_prop.hostRegisterSupported));
    logger.info("Memory pools (mallocAsync):   ", yesNo(_prop.memoryPoolsSupported));
    logger.info("Global L1 cache:              ", yesNo(_prop.globalL1CacheSupported));
    logger.info("Local L1 cache:               ", yesNo(_prop.localL1CacheSupported));
    logger.info("Compute preemption:           ", yesNo(_prop.computePreemptionSupported));
    logger.info("GPUDirect RDMA:               ", yesNo(_prop.gpuDirectRDMASupported));
    logger.info("Timeline semaphore interop:   ", yesNo(_prop.timelineSemaphoreInteropSupported));
    logger.info("Unified function pointers:    ", yesNo(_prop.unifiedFunctionPointers));
}

void CudaDevice::_parseGpuInfo() {
    auto propResult = cudaGetDeviceProperties(&_prop, _deviceId);
    if (propResult != cudaSuccess) {
        return;
    }

    _deviceExists = true;

    _coresPerSM = cudaCoresPerSM(_prop.major, _prop.minor);
    _clockKHz = deviceAttribute(cudaDevAttrClockRate, _deviceId);
    _memoryClockKHz = deviceAttribute(cudaDevAttrMemoryClockRate, _deviceId);
    _residentThreads = _prop.multiProcessorCount * _prop.maxThreadsPerMultiProcessor;
    _totalMemory = _prop.totalGlobalMem;
    _bandwidth = _memoryClockKHz * 1000.0 * (_prop.memoryBusWidth / 8.0) * 2.0 / 1.0e9;
}

std::pair<size_t, size_t> CudaDevice::getMemoryUsage() const {
    size_t freeMemory = 0;
    size_t totalMemory = 0;
    if (cudaMemGetInfo(&freeMemory, &totalMemory) == cudaSuccess) {
        return std::make_pair(freeMemory, totalMemory);
    }
    return std::make_pair(0, 0);
}