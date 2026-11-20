#include "inputController.h"

using namespace vax;

inline vax::KeyCode keyCodeFromScancode(SDL_Scancode scancode) {
    switch (scancode) {
    case SDL_SCANCODE_A:
        return vax::KeyCode::A;
    case SDL_SCANCODE_B:
        return vax::KeyCode::B;
    case SDL_SCANCODE_C:
        return vax::KeyCode::C;
    case SDL_SCANCODE_D:
        return vax::KeyCode::D;
    case SDL_SCANCODE_E:
        return vax::KeyCode::E;
    case SDL_SCANCODE_F:
        return vax::KeyCode::F;
    case SDL_SCANCODE_G:
        return vax::KeyCode::G;
    case SDL_SCANCODE_H:
        return vax::KeyCode::H;
    case SDL_SCANCODE_I:
        return vax::KeyCode::I;
    case SDL_SCANCODE_J:
        return vax::KeyCode::J;
    case SDL_SCANCODE_K:
        return vax::KeyCode::K;
    case SDL_SCANCODE_L:
        return vax::KeyCode::L;
    case SDL_SCANCODE_M:
        return vax::KeyCode::M;
    case SDL_SCANCODE_N:
        return vax::KeyCode::N;
    case SDL_SCANCODE_O:
        return vax::KeyCode::O;
    case SDL_SCANCODE_P:
        return vax::KeyCode::P;
    case SDL_SCANCODE_Q:
        return vax::KeyCode::Q;
    case SDL_SCANCODE_R:
        return vax::KeyCode::R;
    case SDL_SCANCODE_S:
        return vax::KeyCode::S;
    case SDL_SCANCODE_T:
        return vax::KeyCode::T;
    case SDL_SCANCODE_U:
        return vax::KeyCode::U;
    case SDL_SCANCODE_V:
        return vax::KeyCode::V;
    case SDL_SCANCODE_W:
        return vax::KeyCode::W;
    case SDL_SCANCODE_X:
        return vax::KeyCode::X;
    case SDL_SCANCODE_Y:
        return vax::KeyCode::Y;
    case SDL_SCANCODE_Z:
        return vax::KeyCode::Z;
    case SDL_SCANCODE_0:
        return vax::KeyCode::_0;
    case SDL_SCANCODE_1:
        return vax::KeyCode::_1;
    case SDL_SCANCODE_2:
        return vax::KeyCode::_2;
    case SDL_SCANCODE_3:
        return vax::KeyCode::_3;
    case SDL_SCANCODE_4:
        return vax::KeyCode::_4;
    case SDL_SCANCODE_5:
        return vax::KeyCode::_5;
    case SDL_SCANCODE_6:
        return vax::KeyCode::_6;
    case SDL_SCANCODE_7:
        return vax::KeyCode::_7;
    case SDL_SCANCODE_8:
        return vax::KeyCode::_8;
    case SDL_SCANCODE_9:
        return vax::KeyCode::_9;
    case SDL_SCANCODE_LEFT:
        return vax::KeyCode::arrow_left;
    case SDL_SCANCODE_RIGHT:
        return vax::KeyCode::arrow_right;
    case SDL_SCANCODE_UP:
        return vax::KeyCode::arrow_up;
    case SDL_SCANCODE_DOWN:
        return vax::KeyCode::arrow_down;
    default:
        return vax::KeyCode::UNKNOWN;
    }
    return vax::KeyCode::UNKNOWN;
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