#pragma once

namespace vax {
inline bool isAvxSupported() {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_cpu_supports("avx2") > 0;
#elif defined(_MSC_VER)
    int cpuInfo[4];
    __cpuid(cpuInfo, 1);
    bool osUsesXSAVE = (cpuInfo[2] & (1 << 27)) != 0;
    bool cpuHasAVX = (cpuInfo[2] & (1 << 28)) != 0;

    if (osUsesXSAVE && cpuHasAVX) {
        unsigned long long xcrFeatureMask = _xgetbv(_XCR_XFEATURE_ENABLED_MASK);
        bool osSavesYMM = (xcrFeatureMask & 0x6) == 0x6;

        if (osSavesYMM) {
            __cpuid(cpuInfo, 7);
            bool cpuHasAVX2 = (cpuInfo[1] & (1 << 5)) != 0;
            return cpuHasAVX2;
        }
    }
    return false;
#else
    return false;
#endif
}
} // namespace vax