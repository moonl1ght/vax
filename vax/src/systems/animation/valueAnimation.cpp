#include "valueAnimation.h"
#include <algorithm>

using namespace vax;

bool ValueAnimation::updateImpl(const engine::FrameTime& frameTime) {
    if (_isCompleted)
        return true;
    _isStarted = true;
    _currentDuration += frameTime.deltaTime;
    _calculateInterpolatedValue();
    for (auto& animationHandler : _animationHandlers) {
        animationHandler(_currentValue);
    }
    if (_currentDuration >= _duration) {
        _isCompleted = true;
        return true;
    }
    return false;
}

void ValueAnimation::_calculateInterpolatedValue() {
    switch (_function) {
    case AnimationFunction::LINEAR:
        if (_isReversed) {
            auto interpolatedValue = _startValue - ((_currentDuration / _duration) * (_startValue - _endValue));
            interpolatedValue = std::max(interpolatedValue, _endValue);
            _currentValue = interpolatedValue;
        } else {
            auto interpolatedValue = _startValue + (_currentDuration / _duration) * (_endValue - _startValue);
            interpolatedValue = std::min(interpolatedValue, _endValue);
            _currentValue = interpolatedValue;
        }
    }
}

void ValueAnimation::addAnimationHandler(std::function<void(float)> animationHandler) {
    _animationHandlers.push_back(animationHandler);
}