#pragma once

#include <cuda_runtime.h>
#include <string>

namespace vax {

class CudaDevice {
  public:
    CudaDevice(int deviceId)
        : _deviceId(deviceId) {
        _parseGpuInfo();
    }
    ~CudaDevice() = default;

    void printGpuInfo() const;

    bool deviceExists() const { return _deviceExists; }

    int coresPerSM() const { return _coresPerSM; }

    int multiProcessorCount() const { return _prop.multiProcessorCount; }

    int warpSize() const { return _prop.warpSize; }

    int maxThreadsPerBlock() const { return _prop.maxThreadsPerBlock; }

    int maxThreadsPerMultiProcessor() const { return _prop.maxThreadsPerMultiProcessor; }

    int maxBlocksPerMultiProcessor() const { return _prop.maxBlocksPerMultiProcessor; }

    int clockKHz() const { return _clockKHz; }

    int residentThreads() const { return _residentThreads; }

    std::pair<size_t, size_t> getMemoryUsage() const;

    std::string name() const { return _prop.name; }

  private:
    cudaDeviceProp _prop;
    int _deviceId;
    int _coresPerSM;
    int _clockKHz;
    int _memoryClockKHz;
    int _residentThreads;
    float _bandwidth;
    size_t _totalMemory;
    bool _deviceExists = false;

    void _parseGpuInfo();
};

} // namespace vax