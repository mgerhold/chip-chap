#pragma once

#include <..\..\src\emulator\include\chip8\basic_input_source.hpp>
#include <cassert>
#include <unordered_set>

class MockInputSource final : public emulator::BasicInputSource {
public:
    std::unordered_set<emulator::Key> pressed_keys;
    std::vector<emulator::Key> to_be_pressed;

    [[nodiscard]] emulator::Key await_keypress() override {
        return press_next();
    }

    [[nodiscard]] bool is_key_pressed(emulator::Key const key) override {
        return pressed_keys.contains(key);
    }

private:
    [[nodiscard]] emulator::Key press_next() {
        assert(not to_be_pressed.empty());
        auto const next = to_be_pressed.back();
        to_be_pressed.pop_back();
        pressed_keys.insert(next);
        return next;
    }
};
