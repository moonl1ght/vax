#include "systemInfo.h"
#include <fstream>

using namespace vax;

SystemInfo::MemoryStats SystemInfo::getRAMStats() const {
    MemoryStats memoryStats;
    memoryStats.total = 0;
    memoryStats.used = 0;
    memoryStats.free = 0;
    memoryStats.usagePercentage = 0;
#if defined(__linux__)
    size_t rss = 0;
    std::ifstream stream("/proc/self/status");
    std::string line;
    while (std::getline(stream, line)) {
        if (line.rfind("VmRSS:", 0) == 0) {
            sscanf(line.c_str(), "VmRSS: %zu", &rss);
            memoryStats.used = rss / 1024;
        }
    }

    std::ifstream meminfo("/proc/meminfo");
    while (std::getline(meminfo, line)) {
        uint64_t val = 0;
        if (sscanf(line.c_str(), "MemTotal: %lu kB", &val) == 1)
            memoryStats.total = val / 1024;
        if (sscanf(line.c_str(), "MemAvailable: %lu kB", &val) == 1)
            memoryStats.available = val / 1024;
    }

    memoryStats.free = memoryStats.total - memoryStats.used;
    memoryStats.usagePercentage =
        (static_cast<float>(memoryStats.used) / static_cast<float>(memoryStats.total)) * 100.0f;
#endif
    return memoryStats;
}

SystemInfo::GPUStats SystemInfo::getGPUStats() const {
    GPUStats gpuStats;

    gpuStats.name = _cudaEnv.devices()[0].name();
    std::pair<size_t, size_t> memoryUsage = _cudaEnv.devices()[0].getMemoryUsage();
    gpuStats.memoryStats.free = memoryUsage.first / 1024 / 1024;
    gpuStats.memoryStats.total = memoryUsage.second / 1024 / 1024;
    gpuStats.memoryStats.used = gpuStats.memoryStats.total - gpuStats.memoryStats.free;
    if (gpuStats.memoryStats.total == 0) [[unlikely]] {
        gpuStats.memoryStats.usagePercentage = 0.0f;
    } else {
        gpuStats.memoryStats.usagePercentage =
            (static_cast<float>(gpuStats.memoryStats.used) / static_cast<float>(gpuStats.memoryStats.total)) * 100.0f;
    }
    return gpuStats;
}