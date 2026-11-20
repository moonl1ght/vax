#pragma once

#include "cudaEnv.h"
#include <string>

namespace vax {
class SystemInfo final {
  public:
    struct MemoryStats { // in MB
        int total = 0;
        int available = 0;
        int used = 0;
        int free = 0;
        float usagePercentage = 0.0f;
    };

    struct GPUStats {
        std::string name = "";
        MemoryStats memoryStats;
    };

    SystemInfo() {
        _cudaEnv = CudaEnv();
    };

    ~SystemInfo() = default;

    MemoryStats getRAMStats() const;

    GPUStats getGPUStats() const;

  private:
    CudaEnv _cudaEnv;
};
} // namespace vax