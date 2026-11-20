#include "profiler.h"

using namespace vax;

void Profiler::begin(const std::string& name) {
    _startTimes[name] = std::chrono::steady_clock::now();
}

std::chrono::high_resolution_clock::duration Profiler::end(const std::string& name) {
    auto stopTime = std::chrono::steady_clock::now();
    auto duration = stopTime - _startTimes[name];
    _durations[name] = duration;
    return duration;
}