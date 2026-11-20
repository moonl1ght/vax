#pragma once

#include <random>

#define TRUE_RANDOM
#define RANDOM_SEED 42

namespace vax::core {
class RandomGenerator final {
  public:
    static RandomGenerator& getInstance() {
        static RandomGenerator instance;
        return instance;
    }
    RandomGenerator() {
#ifdef TRUE_RANDOM
        _true_random = true;
        _seed = std::random_device()();
        _generator = std::mt19937(_seed);
#else
        _true_random = false;
        _seed = 42;
        _generator = std::mt19937(_seed);
#endif
    }

    ~RandomGenerator() = default;

    int getSeed() const { return _seed; }
    bool trueRandom() const { return _true_random; }

    float uniformFloat(float min = 0.0f, float max = 1.0f) {
        return std::uniform_real_distribution<float>(min, max)(_generator);
    }

    float uniformFloat(std::uniform_real_distribution<float> distribution) { return distribution(_generator); }

    int uniformInt(int min = 0, int max = 1) { return std::uniform_int_distribution<int>(min, max)(_generator); }

    bool uniformBool() { return std::uniform_int_distribution<int>(0, 1)(_generator) == 1; }

    int uniformInt(std::uniform_int_distribution<int> distribution) { return distribution(_generator); }

    std::vector<int> uniformIntDistribution(int size, int min = 0, int max = 1);
    std::vector<float> uniformFloatDistribution(int size, float min = 0.0f, float max = 1.0f);

  private:
    bool _true_random;
    int _seed;
    std::mt19937 _generator;
};
} // namespace vax::core