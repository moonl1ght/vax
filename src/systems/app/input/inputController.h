#pragma once

#include "keyCode.h"
#include "luna.h"

namespace vax {
struct MouseMoveValue {
    glm::vec2 position;
    glm::vec2 delta;
};

struct KeyEvent {
    enum class State {
        DOWN = 0,
        UP = 1,
        HOLD = 2,
    };

    State state;
    KeyCode key;
};

class InputController final {
  public:
    class Observer {
      public:
        InputController* _inputController = nullptr;
    };

    InputController() { _observers.reserve(10); }

    void handleEvent(const SDL_Event& event);

    template <typename TObserver> void addObserver(TObserver* obj) {
        obj->_inputController = this;

        auto mouseMoveWrapper = [](void* inst, const vax::MouseMoveValue& value) {
            static_cast<TObserver*>(inst)->onMouseMove(value);
        };

        auto mouseWheelWrapper = [](void* inst, float delta) { static_cast<TObserver*>(inst)->onMouseWheel(delta); };

        auto keyEventWrapper = [](void* inst, const vax::KeyEvent& keyEvent) {
            static_cast<TObserver*>(inst)->onKeyEvent(keyEvent);
        };

        ObserverSlot slot{static_cast<void*>(obj), mouseMoveWrapper, mouseWheelWrapper, keyEventWrapper};
        if (_observerCount < _observers.size()) {
            _observers[_observerCount] = slot;
        } else {
            _observers.push_back(slot);
        }

        ++_observerCount;
    }

    void removeObserver(void* obj) {
        for (int i = 0; i < _observerCount; ++i) {
            if (_observers[i].instance == obj) {
                _observers[i] = _observers[_observerCount - 1];
                --_observerCount;
                return;
            }
        }
    }

  private:
    struct ObserverSlot {
        void* instance;
        void (*mouseMoveFunc)(void*, const vax::MouseMoveValue&);
        void (*mouseWheelFunc)(void*, float);
        void (*keyEventFunc)(void*, const vax::KeyEvent&);
    };
    std::vector<ObserverSlot> _observers;
    int _observerCount = 0;
    bool _isLeftButtonDown = false;
    std::set<uint32_t> _heldKeys;
};
} // namespace vax