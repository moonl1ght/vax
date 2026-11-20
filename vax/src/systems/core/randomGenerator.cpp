#include "randomGenerator.h"

using namespace vax::core;

std::vector<int> RandomGenerator::uniformIntDistribution(int size, int min, int max) {
    std::vector<int> result(size);
    std::uniform_int_distribution<int> dist(min, max);
    for (int i = 0; i < size; i++) {
        result[i] = dist(_generator);
    }
    return result;
}

std::vector<float> RandomGenerator::uniformFloatDistribution(int size, float min, float max) {
    std::vector<float> result(size);
    std::uniform_real_distribution<float> dist(min, max);
    for (int i = 0; i < size; i++) {
        result[i] = dist(_generator);
    }
    return result;
}