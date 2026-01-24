#ifndef GAME_INPUTSYS_H
#define GAME_INPUTSYS_H

#include <vector>
#include <string>
#include <functional>
#include <unordered_map>

#include "raylib.h"

#include "vec.h"

enum GamepadId {
    KeyboardAndMouse = 0,
    Gamepad1,
    Gamepad2,
    Gamepad3,
    Gamepad4,
};

struct BooleanGamepadBinding {
    std::function<bool()> keyboard;
    std::function<bool(int)> gamepad;

    [[nodiscard]] bool test(const GamepadId id) const {
        if (id == KeyboardAndMouse) return keyboard();
        return gamepad(static_cast<int>(id) - 1); // gamepad1 is id 0.
    }
};

struct AxisGamepadBinding {
    std::function<float()> keyboard;
    std::function<float(int)> gamepad;

    [[nodiscard]] float test(const GamepadId id) const {
        if (id == KeyboardAndMouse) return keyboard();
        return gamepad(static_cast<int>(id) - 1); // gamepad1 is id 0.
    }
};

struct Vec2GamepadBinding {
    std::function<Vec2()> keyboard;
    std::function<Vec2(int)> gamepad;

    [[nodiscard]] Vec2 test(const GamepadId id) const {
        if (id == KeyboardAndMouse) return keyboard();
        return gamepad(static_cast<int>(id) - 1); // gamepad1 is id 0.
    }
};

class Input {
    // std::vector<GamepadId> freeGamepadSlots = {KeyboardAndMouse, Gamepad1, Gamepad2, Gamepad3, Gamepad4}; // -1 for keyboard
    std::unordered_map<std::string, BooleanGamepadBinding> booleanBindings;
    std::unordered_map<std::string, AxisGamepadBinding> axisBindings;
    std::unordered_map<std::string, Vec2GamepadBinding> vec2Bindings;

public:
    void bindBoolean(const std::string& bindName, const BooleanGamepadBinding& binding) {
        booleanBindings[bindName] = binding;
    }

    void bindAxis(const std::string& bindName, const AxisGamepadBinding& binding) {
        axisBindings[bindName] = binding;
    }

    void bindVector2(const std::string& bindName, const Vec2GamepadBinding& binding) {
        vec2Bindings[bindName] = binding;
    }

    [[nodiscard]] bool testBooleanBind(const GamepadId id, const std::string& bindName) const {
        return booleanBindings.at(bindName).test(id);
    }

    [[nodiscard]] float testAxisBind(const GamepadId id, const std::string& bindName) const {
        return axisBindings.at(bindName).test(id);
    }

    [[nodiscard]] Vec2 testVec2Bind(const GamepadId id, const std::string& bindName) const {
        return vec2Bindings.at(bindName).test(id);
    }
};

class GamepadInterface {

};

#endif //GAME_INPUTSYS_H