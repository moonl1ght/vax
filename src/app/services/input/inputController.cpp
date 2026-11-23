#include "inputController.h"

using namespace vax::app;

inline KeyCode keyCodeFromScancode(SDL_Scancode scancode) {
    switch (scancode) {
    case SDL_SCANCODE_A:
        return KeyCode::A;
    case SDL_SCANCODE_B:
        return KeyCode::B;
    case SDL_SCANCODE_C:
        return KeyCode::C;
    case SDL_SCANCODE_D:
        return KeyCode::D;
    case SDL_SCANCODE_E:
        return KeyCode::E;
    case SDL_SCANCODE_F:
        return KeyCode::F;
    case SDL_SCANCODE_G:
        return KeyCode::G;
    case SDL_SCANCODE_H:
        return KeyCode::H;
    case SDL_SCANCODE_I:
        return KeyCode::I;
    case SDL_SCANCODE_J:
        return KeyCode::J;
    case SDL_SCANCODE_K:
        return KeyCode::K;
    case SDL_SCANCODE_L:
        return KeyCode::L;
    case SDL_SCANCODE_M:
        return KeyCode::M;
    case SDL_SCANCODE_N:
        return KeyCode::N;
    case SDL_SCANCODE_O:
        return KeyCode::O;
    case SDL_SCANCODE_P:
        return KeyCode::P;
    case SDL_SCANCODE_Q:
        return KeyCode::Q;
    case SDL_SCANCODE_R:
        return KeyCode::R;
    case SDL_SCANCODE_S:
        return KeyCode::S;
    case SDL_SCANCODE_T:
        return KeyCode::T;
    case SDL_SCANCODE_U:
        return KeyCode::U;
    case SDL_SCANCODE_V:
        return KeyCode::V;
    case SDL_SCANCODE_W:
        return KeyCode::W;
    case SDL_SCANCODE_X:
        return KeyCode::X;
    case SDL_SCANCODE_Y:
        return KeyCode::Y;
    case SDL_SCANCODE_Z:
        return KeyCode::Z;
    case SDL_SCANCODE_0:
        return KeyCode::_0;
    case SDL_SCANCODE_1:
        return KeyCode::_1;
    case SDL_SCANCODE_2:
        return KeyCode::_2;
    case SDL_SCANCODE_3:
        return KeyCode::_3;
    case SDL_SCANCODE_4:
        return KeyCode::_4;
    case SDL_SCANCODE_5:
        return KeyCode::_5;
    case SDL_SCANCODE_6:
        return KeyCode::_6;
    case SDL_SCANCODE_7:
        return KeyCode::_7;
    case SDL_SCANCODE_8:
        return KeyCode::_8;
    case SDL_SCANCODE_9:
        return KeyCode::_9;
    case SDL_SCANCODE_LEFT:
        return KeyCode::arrow_left;
    case SDL_SCANCODE_RIGHT:
        return KeyCode::arrow_right;
    case SDL_SCANCODE_UP:
        return KeyCode::arrow_up;
    case SDL_SCANCODE_DOWN:
        return KeyCode::arrow_down;
    default:
        return KeyCode::UNKNOWN;
    }
    return KeyCode::UNKNOWN;
}

void InputController::handleEvent(const SDL_Event& event) {
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            _isLeftButtonDown = true;
        }
    }
    if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            _isLeftButtonDown = false;
        }
    }
    if (event.type == SDL_EVENT_MOUSE_WHEEL) {
        for (int i = 0; i < _observerCount; ++i) {
            _observers[i].mouseWheelFunc(_observers[i].instance, event.wheel.y);
        }
    }
    if (event.type == SDL_EVENT_KEY_DOWN) {
        KeyEvent keyEvent;
        auto key = keyCodeFromScancode(event.key.scancode);
        if (key == KeyCode::UNKNOWN) {
            return;
        }
        uint32_t keyValue = static_cast<uint32_t>(key);
        if (_heldKeys.find(keyValue) == _heldKeys.end()) {
            _heldKeys.insert(keyValue);
            keyEvent = {.state = KeyEvent::State::DOWN, .key = key};
        } else {
            keyEvent = {.state = KeyEvent::State::HOLD, .key = key};
        }
        for (int i = 0; i < _observerCount; ++i) {
            _observers[i].keyEventFunc(_observers[i].instance, keyEvent);
        }
    }
    if (event.type == SDL_EVENT_KEY_UP) {
        auto key = keyCodeFromScancode(event.key.scancode);
        if (key == KeyCode::UNKNOWN) {
            return;
        }
        uint32_t keyValue = static_cast<uint32_t>(key);
        if (_heldKeys.find(keyValue) == _heldKeys.end()) {
            return;
        }
        _heldKeys.erase(keyValue);
        KeyEvent keyEvent = {.state = KeyEvent::State::UP, .key = key};
        for (int i = 0; i < _observerCount; ++i) {
            _observers[i].keyEventFunc(_observers[i].instance, keyEvent);
        }
    }
    if (_isLeftButtonDown) {
        if (event.type == SDL_EVENT_MOUSE_MOTION) {
            for (int i = 0; i < _observerCount; ++i) {
                MouseMoveValue value = {
                    .position = glm::vec2(event.motion.x, event.motion.y),
                    .delta = glm::vec2(event.motion.xrel, event.motion.yrel)
                };
                _observers[i].mouseMoveFunc(_observers[i].instance, value);
            }
        }
    }
}