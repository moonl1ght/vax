#pragma once

namespace vax::rl {
struct QLearningConfig final {
    double learningRate = 0.1;
    double gamma = 0.9;
    double epsilon = 0.3;
    int episodes = 100;
};
} // namespace vax::rl