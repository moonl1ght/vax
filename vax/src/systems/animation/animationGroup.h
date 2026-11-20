#pragma once

#include "animation.h"
#include "frameTime.h"
#include "valueAnimation.h"
#include <optional>
#include <variant>
#include <vector>

namespace vax {
class AnimationGroup final : public Animation<AnimationGroup> {
  public:
    using AnimationType = std::variant<AnimationGroup, ValueAnimation>;

    enum class Mode {
        SERIAL,
        PARALLEL,
    };

    AnimationGroup(Mode mode)
        : _mode(mode) {};

    ~AnimationGroup() = default;

    bool updateImpl(const engine::FrameTime& frameTime) {
        if (_animations.empty()) {
            return true;
        }
        bool isCompleted = false;
        switch (_mode) {
        case Mode::SERIAL:
            isCompleted = _playSerial(frameTime);
            break;
        case Mode::PARALLEL:
            isCompleted = _playParallel(frameTime);
            break;
        }
        if (isCompleted && _callback.has_value()) {
            (*_callback)();
        }
        return isCompleted;
    }

    void pushAnimation(ValueAnimation animation) { _animations.push_back(std::move(animation)); }

    void setMode(Mode mode) { _mode = mode; }

    void setCompletionCallback(std::function<void()> callback) { _callback = std::make_optional(callback); }

  private:
    Mode _mode;
    std::vector<AnimationType> _animations;
    int _currentAnimationIndex = 0;
    std::optional<std::function<void()>> _callback = std::nullopt;

    bool _playSerial(const engine::FrameTime& frameTime) {
        if (_currentAnimationIndex >= _animations.size()) {
            return true;
        }
        auto& animation = _animations[_currentAnimationIndex];
        bool isCompleted = false;
        std::visit([&](auto& animation) { isCompleted = animation.update(frameTime); }, animation);

        if (isCompleted) {
            ++_currentAnimationIndex;
            if (_currentAnimationIndex >= _animations.size()) {
                return true;
            }
        }

        return false;
    }

    bool _playParallel(const engine::FrameTime& frameTime) {
        bool allCompleted = true;
        for (auto& animation : _animations) {
            bool isCompleted = false;
            std::visit([&](auto& animation) { isCompleted = animation.update(frameTime); }, animation);
            allCompleted &= isCompleted;
        }
        return allCompleted;
    }
};
} // namespace vax